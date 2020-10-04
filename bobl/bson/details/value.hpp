// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/details/sequence.hpp"
#include "bobl/bson/details/numeric.hpp"
#include "bobl/bson/details/options.hpp"
#include "bobl/bson/adapter.hpp"
#include "bobl/bson/details/header.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/float.hpp"
#include "bobl/utility/has_is.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/utils.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/utility/timepoint.hpp"
#include "bobl/bobl.hpp"
#include <boost/format.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <tuple>
#include <type_traits>


namespace bobl{ namespace bson { namespace flyweight{ 
		
namespace details{ 

template<typename T, typename U = ObjectHeader>
using HasIs = bobl::utility::HasIs<T, U>;

class EmbeddedDocument
{
protected:
	EmbeddedDocument(ObjectHeader&& header) : header_{ std::move(header) } {}

	std::size_t size() const
	{
		auto value = *reinterpret_cast<std::uint32_t const*>(header_.value());
		return std::size_t(boost::endian::little_to_native(value));
	}
	template<typename Iterator>
	boost::iterator_range<Iterator> raw_range() const
	{
		static_assert(std::is_pointer<Iterator>::value && sizeof(typename std::iterator_traits<Iterator>::value_type) == sizeof(std::uint8_t), "at the moment only sizeof(std::uint8_t) popinters supported");
		auto begin = header_.position();
		auto end = header_.value() + size();
		return boost::make_iterator_range(Iterator(begin), Iterator(end));
	}
public:
	diversion::string_view name() const { return header_.name(); }

	boost::iterator_range<bobl::bson::flyweight::Iterator> value() const
	{
		auto begin = header_.value();
		return boost::make_iterator_range(begin + /*object size*/ sizeof(std::uint32_t) / sizeof(std::uint8_t), begin + size() - /*trailing x00*/ 1);
	}

	template<bobl::bson::Type Type>
	static EmbeddedDocument decode_as(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		auto header = ObjectHeader{ begin, end };
		begin = header.validate<Type>(end);
		return { std::move(header) };
	}
private:
	ObjectHeader header_;
};

template<typename T, typename Options, typename Enabled = boost::mpl::true_>
class ValueHandler {};


template<typename T>
struct IsValueHandler : std::false_type {};

template<typename ...Args>
struct IsValueHandler<ValueHandler<Args...>> : std::true_type {};

template</*typename Iterator,*/ typename Options>
class ValueHandler<bobl::flyweight::lite::Any</*Iterator*/bobl::bson::flyweight::Iterator>, Options, boost::mpl::true_> : public ValueHandlerBase
{
	using ValueType = bobl::flyweight::lite::Any<bobl::bson::flyweight::Iterator>;
	ValueHandler(ObjectHeader&& header, bobl::bson::flyweight::Iterator end) : ValueHandlerBase{ std::move(header) }, end_{std::move(end)} {}
public:
	ValueType operator()() const { return { position(), end_ }; }
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		begin = header.validate(end);
		return ValueHandler(std::move(header), begin);
	}
	static bool is(details::ObjectHeader const& header) { return true; }
private:
	bobl::bson::flyweight::Iterator end_;
};


 template</*typename Iterator,*/ typename Options>
 class ValueHandler<bobl::flyweight::lite::Array</*Iterator*/bobl::bson::flyweight::Iterator>, Options, boost::mpl::true_> : EmbeddedDocument
 {
	 static constexpr bobl::bson::Type BsonType = bobl::bson::Array;
	 using EmbeddedDocument::EmbeddedDocument;
 public:
	 using EmbeddedDocument::name;
	 bobl::flyweight::lite::Array</*Iterator*/bobl::bson::flyweight::Iterator> operator()() const
	 {
		 auto const& val = raw_range</*Iterator*/bobl::bson::flyweight::Iterator>();
		 return { val.begin(), val.end() };
	 }

	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType>(end);
		 return ValueHandler(std::move(header));
	 }
 };

 template</*typename Iterator,*/ typename Options>
 class ValueHandler<bobl::flyweight::lite::Object</*Iterator*/bobl::bson::flyweight::Iterator>, Options, boost::mpl::true_> : EmbeddedDocument
 {
	 static constexpr bobl::bson::Type BsonType = bobl::bson::EmbeddedDocument;
	 using EmbeddedDocument::EmbeddedDocument;
 public:
	 using EmbeddedDocument::name;
	 bobl::flyweight::lite::Object</*Iterator*/bobl::bson::flyweight::Iterator> operator()() const
	 {
		 auto const& val = raw_range</*Iterator*/bobl::bson::flyweight::Iterator>();
		 return { val.begin(), val.end() };
	 }

	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType>(end);
		 return ValueHandler(std::move(header));
	 }
 };

template<typename Options>
class ValueHandler<bool, Options, boost::mpl::true_> : public SimpleValueHandlerBase<bobl::bson::Type::Bool>
{
	using Base = SimpleValueHandlerBase<bobl::bson::Type::Bool>;
	ValueHandler(Base&& base) : Base{std::move(base)} {}
public:
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		return ValueHandler{ Base::decode(std::move(header), begin, end) };
	}
	bool operator()() const { return *value() != 0; }
};

template<typename Duration, typename Options>
class ValueHandler<std::chrono::time_point<std::chrono::system_clock, Duration>, Options, boost::mpl::true_> : public SimpleValueHandlerBase<bobl::bson::Type::UTCDateTime>
{
	using Base = SimpleValueHandlerBase<bobl::bson::Type::UTCDateTime>;
	ValueHandler(Base&& base) : Base{ std::move(base) } {}
public:
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		return ValueHandler{ Base::decode(std::move(header), begin, end) };
	}
	std::chrono::time_point<std::chrono::system_clock, Duration> operator()() const
	{ 
		return bobl::utility::make_timpoint<Duration>(std::chrono::duration<int64_t, typename std::chrono::milliseconds::period>{boost::endian::little_to_native(*reinterpret_cast<const int64_t*> (value()))});
	}
};

template<typename T, typename Options>
class ValueHandler<T, Options, boost::mpl::bool_<std::is_integral<T>::value>  >
	 : public std::conditional<bobl::utility::options::Contains<typename bobl::bson::EffectiveOptions<T, Options>::type, bobl::options::RelaxedIntegers>::value, RelaxedIntegerHandler<T,Options>, StrictIntegerHandler<T, Options>>::type
{
	 using Base = typename std::conditional<bobl::utility::options::Contains<typename bobl::bson::EffectiveOptions<T, Options>::type, bobl::options::RelaxedIntegers>::value, RelaxedIntegerHandler<T, Options>, StrictIntegerHandler<T, Options>>::type;
	 ValueHandler(Base&& base) :Base{ std::move(base) } {}
public:
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		return { Base::decode(std::move(header), begin,end) };
	}
};

template<typename T, typename Options>
class ValueHandler<T, Options, boost::mpl::bool_<std::is_floating_point<T>::value> >
	: public SimpleValueHandlerBase<bobl::bson::Type::Double>
{
	using Base = SimpleValueHandlerBase<bobl::bson::Type::Double>;
	ValueHandler(Base&& base) : Base{std::move(base)} {}
public:
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		return ValueHandler{ Base::decode(std::move(header), begin, end) };
	}
	T operator()() const
	{
		auto val = boost::endian::little_to_native(*reinterpret_cast<const uint64_t*> (value()));
		return T(bobl::utility::FloatConverter<sizeof(std::uint64_t)>{}(val));
	}
};

 template<typename Options>
 class ValueHandler<std::string, Options, boost::mpl::true_> : public ValueHandlerBase
 {
	 static constexpr bobl::bson::Type BsonType = bobl::bson::Utf8String;
	 using ValueHandlerBase::ValueHandlerBase;
 public:
	 std::size_t length() const { return boost::endian::little_to_native(*reinterpret_cast<const uint32_t*> (value())); }
	 std::string operator()() const { return { reinterpret_cast<char const*>(value() + sizeof(std::uint32_t) /*size*/), length() - 1 }; }
	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType>(end);
		 return ValueHandler(std::move(header));
	 }
	 static bool is(details::ObjectHeader const& header) { return header.type() == BsonType; }
 };

 class Binary : public ValueHandlerBase
 {
 protected:
	 using ValueHandlerBase::ValueHandlerBase; 
	 std::size_t size() const { return boost::endian::little_to_native(*reinterpret_cast<const uint32_t*> (ValueHandlerBase::value())); }
	 std::uint8_t const* value() const { return ValueHandlerBase::value() + sizeof(std::uint32_t) /*size*/ + sizeof(std::uint8_t) /*subtype*/; }
	 bobl::bson::BinSubtype subtype() const { return bobl::bson::BinSubtype(*(ValueHandlerBase::value() + sizeof(std::uint32_t))); }
 };

 template<typename T, typename Options>
 class ValueHandler<std::vector<T>, Options, typename bobl::utility::IsByteType<T, Options>::type> : public Binary
 {
	 static constexpr bobl::bson::Type BsonType = bobl::bson::Binary;
	 using Binary::Binary;
 public:
	 std::vector<T> operator()() const 
	 { 
		 auto begin = value();
		 return { begin, begin + size() }; 
	 }
	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType>(end);
		 return ValueHandler(std::move(header));
	 }
	 static bool is(details::ObjectHeader const& header) { return header.type() == BsonType; }
 };

 template<typename Options>
 class ValueHandler<boost::uuids::uuid, Options, boost::mpl::true_> : public Binary
 {
	 static constexpr bobl::bson::Type BsonType = bobl::bson::Binary;
	 ValueHandler(ObjectHeader&& header) : Binary{ std::move(header) } 
	 {
		 auto stype = subtype();
		 if(stype != bobl::bson::Uuid && stype != bobl::bson::UuidOld)
			 throw bobl::IncorrectObjectType{ str(boost::format("BSON object %1% has unexpected subtype type : %2% (%3$#x) insted of expected: %4%") % name() % to_string(stype) % int(stype) % to_string(bobl::bson::Uuid)) };
	 }
 public:
	 boost::uuids::uuid operator()() const
	 { 
		 boost::uuids::uuid res;
		 if(size() != sizeof(res.data))
			 throw bobl::InputToShort(str(boost::format("invalid data size of BSON \"%1%\" (%2 insted of expected %3 ") % to_string(subtype()) % size() % sizeof(res.data)));
		 auto begin = value();
		 std::copy(begin, begin + size(), res.data);
		 return res;
	 }
	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType>(end);
		 return { std::move(header) };
	 }
//"TODO: should be extended to check subtype")
	 static bool is(details::ObjectHeader const& header) { return header.type() == BsonType; }
 };


 template<typename T, typename Options>
 class ValueHandler<T, Options, typename boost::fusion::traits::is_sequence<T>::type> : EmbeddedDocument
 {
	 using HeterogeneousArray = bobl::utility::IsHeterogeneousArraySequence<bobl::bson::NsTag, T, Options>;
	 using BsonType = typename std::conditional<HeterogeneousArray::value,
												std::integral_constant<bobl::bson::Type, bobl::bson::Array>,
												std::integral_constant<bobl::bson::Type, bobl::bson::EmbeddedDocument>>::type;

	 using EmbeddedDocument::EmbeddedDocument;
 public:
	 using EmbeddedDocument::name;
	 T operator()() const
	 {
		 auto const& val = value();
		 return SequenceHandler<T, HeterogeneousArray::value, typename bobl::bson::EffectiveOptions<T, Options>::type>::decode(val.begin(), val.end());
	 }

	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType::value>(end);
		 return ValueHandler(std::move(header));
	 }
	 //	static bool is(details::ObjectHeader const& header) { return header.type() == BsonType::value; }
 };

 
 template<typename T, typename Options>
 class ValueHandler<std::vector<T>, Options, typename boost::mpl::not_<bobl::utility::IsByteType<T, Options>>::type> : EmbeddedDocument
 {
	 static constexpr bobl::bson::Type BsonType = bobl::bson::Array;
	 using EmbeddedDocument::EmbeddedDocument;
 public:
	 using EmbeddedDocument::name;
	 std::vector<T> operator()() const
	 {	
		 std::vector<T> res;
		 auto const& val = value();
		 auto end = val.end();
		 for (auto begin = val.begin(); begin != end;)
			 res.emplace_back(NameValue<T, Options>::decode(begin, end).value());
		 return res;
	 }

	 static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	 {
		 begin = header.validate<BsonType>(end);
		 return ValueHandler(std::move(header));
	 }
 };


template<typename T, typename Options>
class EffectiveValueHandler
{
	using ValueType = typename std::conditional<bobl::utility::Adaptable<T, bobl::bson::Adapter<T>>::value, bobl::bson::Adapter<T>, T>::type;
public:
	using type = ValueHandler<ValueType, Options>;
};

template<typename ...Params, typename Options>
class ValueHandler<diversion::variant<Params...>, Options, boost::mpl::true_> 
{
	static_assert(sizeof...(Params) != 0, "empty variant makes no sense here");
	using TypesTuple = typename boost::mpl::transform<std::tuple<Params...>, EffectiveValueHandler<boost::mpl::placeholders::_1, Options>>::type;
	using NestedValueHandler = typename bobl::utility::MakeVariant<TypesTuple>::type;

	using ValueType = diversion::variant<Params...>;
	struct ValueVisitor : boost::static_visitor<ValueType>
	{
		ValueVisitor() = default;
		template<typename T>
		ValueType operator()(T handler) const { return handler(); }
	};

	struct NameVisitor : boost::static_visitor<diversion::string_view>
	{
		NameVisitor() = default;
		template<typename T>
		ValueType operator()(T handler) const { return handler.name(); }
	};

	ValueHandler(NestedValueHandler&& nested) : nested_{ std::move(nested) } {}
public:
	diversion::string_view name() const { return diversion::visit(NameVisitor{}, nested_); }
	ValueType operator()() const {	return diversion::visit(ValueVisitor{}, nested_);	}
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		if (begin == end)
		{
			auto param_list = bobl::utility::type_name<Params...>();
			throw bobl::InputToShort{ str(boost::format("not enought data to decode BSON variant <%1%>") % param_list) };
		}
		return { try_decode<Params...>(std::move(header), begin, end) };
	}
private:
	template<typename ...Args>
	static auto try_decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& /*begin*/, bobl::bson::flyweight::Iterator /*end*/)
		-> typename std::enable_if< sizeof...(Args) == 0, NestedValueHandler>::type
	{
		auto type = header.type();
		auto param_list = bobl::utility::type_name<Params...>();
		throw bobl::IncorrectObjectType{ str(boost::format("BSON value has unexpected type : %1% (%2$#x) insted of expected variant <%3%>") 
																	% to_string(type) % int(type) % param_list) };	
	}

	template<typename T, typename ...Args>
	static NestedValueHandler try_decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		static_assert(!bobl::utility::IsOptional<T>::value, "there is no much sense in having optional as one of variant values");
		assert(begin != end);
		return try_decode_<typename EffectiveValueHandler<T, Options>::type, Args...>(std::move(header), begin, end);
	}

	template<typename T, typename ...Args>
	static auto try_decode_(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
		->	typename std::enable_if<HasIs<T>::value, NestedValueHandler>::type
	{
		return T::is(header)
			? NestedValueHandler{ T::decode(std::move(header), begin, end) }
			: try_decode<Args...>(std::move(header), begin, end);
	}

	template<typename T, typename ...Args>
	static auto try_decode_(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
		->	typename std::enable_if<!HasIs<T>::value, NestedValueHandler>::type
	{
		auto i = begin;
		try
		{
			return NestedValueHandler{ T::decode(std::move(header), begin, end) };
		}
		catch (bobl::IncorrectObjectType&)
		{
		}
		begin = i;
		return try_decode<Args...>(std::move(header), begin, end);
	}
private:
	NestedValueHandler nested_;
};

template<typename T, typename ...Options>
class ValueHandler<bobl::bson::Adapter<T>, bobl::Options<Options...>, boost::mpl::true_>
{
	using Adaptee = ValueHandler<typename bobl::bson::Adapter<T>::type, typename bson::EffectiveOptions<T, Options...>::type>;
	ValueHandler(Adaptee&& addaptee) : addaptee_{ std::move(addaptee) } {}
public:
	diversion::string_view name() const { return addaptee_.name(); }
	T operator()() const { return bobl::bson::Adapter<T>{}(addaptee_()); }
	template<typename Header>
	static auto is(Header/*ObjectHeader*/ const& header) -> typename std::enable_if<HasIs<Adaptee, Header>::value, bool>::type
	{
		return Adaptee::is(header);
	}
	static ValueHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		return { Adaptee::decode(std::move(header), begin, end) };
	}
private:
	Adaptee addaptee_;
};

} /*namespace details*/ } /*namespace flyweight*/ } /*namespace bson*/ } /*namespace bobl*/

