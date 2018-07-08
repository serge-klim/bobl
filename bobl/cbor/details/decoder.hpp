// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/cbor/details/utility.hpp"
#include "bobl/cbor/details/options.hpp"
#include "bobl/cbor/options.hpp"
#include "bobl/cbor/adapter.hpp"
#include "bobl/cbor/cbor.hpp"
#include "bobl/utility/timepoint.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/has_is.hpp"
#include "bobl/utility/utils.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/decoders.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/core/ignore_unused.hpp>
#include <chrono>
#include <algorithm>
#include <vector>
#include <string>
#include <type_traits>
#include <cstdint>
#include <cassert>


namespace bobl{ namespace cbor { namespace decoder { namespace details {


template<typename T, typename Options, typename Enabled = boost::mpl::true_>
class Handler{};

template<typename Options>
class Handler<bool, Options, boost::mpl::true_>
{
public:
	constexpr static bool is(cbor::Type type)
	{
		return (type == bobl::cbor::True) || (type == bobl::cbor::False);
	}

	template<typename Iterator>
	static bool decode(Iterator& begin, Iterator end)
	{
		bool res = false;
		bobl::cbor::utility::decode::validate<cbor::MajorType::SimpleValue>(begin, end);
		switch (auto type = bobl::cbor::utility::decode::type(*begin))
		{
			case bobl::cbor::False:
				res = false;
				break;
			case bobl::cbor::True:
				res = true;
				break;
			default:
				throw bobl::IncorrectObjectType{ str(boost::format("CBOR value has unexpected type : %1% (%2$#x) insted of expected: bool(False/True)") % to_string(type) % int(type) ) };
		}
		assert(is(bobl::cbor::utility::decode::type(*begin)) && "cbor bool::is is broken");
		++begin;
		return res;
	}
};

template<typename T, typename Options>
class Handler<std::basic_string<T>, Options, boost::mpl::true_> 
{
public:
	constexpr static bool is(cbor::Type type)
	{
		return bobl::cbor::utility::decode::major_type(type) == cbor::MajorType::TextString;
	}

	template<typename Iterator> 
	static std::basic_string<T> decode(Iterator& begin, Iterator end)
	{
		bobl::cbor::utility::decode::validate<cbor::MajorType::TextString>(begin, end);
		assert(is(bobl::cbor::utility::decode::type(*begin)) && "cbor string::is is broken");
		return bobl::cbor::utility::decode::string<std::basic_string<T>>(begin, end);
	}
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, typename bobl::utility::IsByteType<T, Options>::type>
{
public:
	constexpr static bool is(cbor::Type type)
	{
		return bobl::cbor::utility::decode::major_type(type) == cbor::MajorType::ByteString;
	}
	template<typename Iterator>
	static std::vector<T> decode(Iterator& begin, Iterator end)
	{
		bobl::cbor::utility::decode::validate<cbor::MajorType::ByteString>(begin, end);
		assert(is(bobl::cbor::utility::decode::type(*begin)) && "cbor byte string::is is broken");
		return bobl::cbor::utility::decode::string<std::vector<T>>(begin, end);
	}
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, typename boost::mpl::not_<bobl::utility::IsByteType<T, Options>>::type>
{
	static_assert(!bobl::utility::IsOptional<T>::value, "there is no much sense in having Array of optional(s)");
public:
	constexpr static bool is(cbor::Type type)
	{
		return bobl::cbor::utility::decode::major_type(type) == cbor::MajorType::Array;
	}

	template<typename Iterator>
	static std::vector<T> decode(Iterator& begin, Iterator end)
	{
		std::vector<T> res;
		bobl::cbor::utility::decode::validate<cbor::MajorType::Array>(begin, end);
		assert(is(bobl::cbor::utility::decode::type(*begin)) && "cbor array::is is broken");
		auto len = bobl::cbor::utility::decode::lenght(begin, end);
		if (len != bobl::cbor::utility::decode::IndefiniteLenght)
		{
			res.reserve(len);
			while (len-- != 0)
				res.emplace_back(Handler<T, Options>::decode(begin, end));
		}
		else
		{
			for (;;)
			{
				if (begin == end)
					throw bobl::InvalidObject{"not enough data provided to decode indefinite lenght CBOR array"};
				if (*begin == cbor::Break)
					break;
				res.emplace_back(Handler<T, Options>::decode(begin, end));
			}
		}
		return res;
	}
};


template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_integral<T>::value>>
{
public:
	constexpr static bool is(cbor::MajorType type)
	{
		return utility::decode::is_integer(type);
	}

	constexpr static bool is(cbor::Type type)
	{
		return is(bobl::cbor::utility::decode::major_type(type));
	}

	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		if(begin == end)
			throw bobl::InputToShort{ "not enought data to decode CBOR value type" };
		auto type = bobl::cbor::utility::decode::major_type(*begin);
		if(!is(type))
			throw bobl::IncorrectObjectType{ str(boost::format("CBOR value has unexpected type : %1$#x insted of expected integer type") % int(type)) };
		auto val = bobl::cbor::utility::decode::integer(begin, end);
		auto res = T(val);
		if (std::uint64_t(res) != val)
			throw bobl::IncorrectObjectType{ str(boost::format("%1% bit integer is short to hold %2% ") % (8*sizeof(T)) % (type == bobl::cbor::MajorType::NegativeInt ? -res : res)  ) };
		return type == bobl::cbor::MajorType::NegativeInt ? (-res - 1) : res;
	}
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_floating_point<T>::value>>
{
public:
	constexpr static bool is(cbor::Type type)
	{ 
		return utility::decode::is_float(type);
	}

	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		bobl::cbor::utility::decode::validate<cbor::MajorType::Float>(begin, end);
		assert(is(bobl::cbor::utility::decode::type(*begin)) && "cbor float::is is broken");
		return bobl::cbor::utility::decode::floating_point<T>(begin, end);;
	}
};

class Name
{
public:
	explicit Name(std::string name) : name_{ name } {}
	diversion::string_view operator()() const { return name_; }
private:
	std::string name_;
};


template<typename T, typename Options>
class Handler<bobl::flyweight::lite::utility::Any<T>, Options, boost::mpl::true_>
{
public:
	constexpr static bool is(cbor::Type type) { return true; }
	template<typename Iterator>
	static bobl::flyweight::lite::Any<Iterator> decode(Iterator& begin, Iterator end)
	{
		auto i = begin;
		switch (auto type = bobl::cbor::utility::decode::major_type(*begin))
		{
			case bobl::cbor::MajorType::UnsignedInt:
			case bobl::cbor::MajorType::NegativeInt:
			case bobl::cbor::MajorType::SimpleValue:
				bobl::cbor::utility::decode::integer(begin, end);
				break;
			case bobl::cbor::MajorType::ByteString:
			case bobl::cbor::MajorType::TextString:
			{
				auto len = bobl::cbor::utility::decode::lenght(begin, end);
				if (len != bobl::cbor::utility::decode::IndefiniteLenght)
				{
					if (decltype(len)(std::distance(begin, end)) < len)
						throw bobl::InputToShort(str(boost::format("not enought data to decode CBOR \"%1%\"") % to_string(type)));
					std::advance(begin, len);
				}
				else
				{
					begin = std::find(begin, end, cbor::Break);
					if(begin == end)
						throw bobl::InvalidObject(str(boost::format("break is missing for indefinite lenght \"%1%\"") % to_string(type)));
				}
				break;
			}
			case bobl::cbor::MajorType::Array:
				decode_array(begin, end, &Handler::decode<Iterator>);
				break;
			case bobl::cbor::MajorType::Dictionary:
				decode_array(begin, end, &Handler::decode_pair<Iterator>);
				break;
			case bobl::cbor::MajorType::Tag:
				bobl::cbor::utility::decode::integer(begin, end);
				decode(begin, end);
				break;
			default:
				throw bobl::InvalidObject(str(boost::format("CBOR bobl::flyweight::lite::Any decoder does not supported \"%1%\" yet!") % to_string(type)));
		}
		return {i, begin};
	}

	template<typename Iterator>
	static void decode_array(Iterator& begin, Iterator end, bobl::flyweight::lite::Any<Iterator>(*value_decoder)(Iterator& begin, Iterator end))
	{
		decode_array(bobl::cbor::utility::decode::lenght(begin, end), begin, end, value_decoder);
	}

	template<typename Iterator>
	static void decode_array(std::uint64_t len, Iterator& begin, Iterator end, bobl::flyweight::lite::Any<Iterator>(*value_decoder)(Iterator& begin, Iterator end))
	{
		if (len != bobl::cbor::utility::decode::IndefiniteLenght)
		{
			while (len--!=0)
				(*value_decoder)(begin, end);
		}
		else
		{
			for (;;)
			{
				if (begin == end)
					throw bobl::InvalidObject(str(boost::format("not enough data provided to decode indefinite lenght CBOR \"%1%\"") % 
							(value_decoder== &Handler::decode_pair<Iterator> ? "dictionary" : "array")));
				if (*begin == cbor::Break)
					break;
				(*value_decoder)(begin, end);
			}
		}
	}

	template<typename Iterator>
	static bobl::flyweight::lite::Any<Iterator> decode_pair(Iterator& begin, Iterator end)
	{
		auto i = begin;
		//bobl::cbor::utility::decode::validate<cbor::MajorType::TextString>(begin, end);
		decode(begin, end); // name
		decode(begin, end); // value
		return { i, begin};
	}
};

template<typename T, typename Options>
class Handler<bobl::flyweight::lite::utility::Array<T>, Options, boost::mpl::true_>
{
public:
	constexpr static bool is(cbor::Type type)
	{
		return bobl::cbor::utility::decode::major_type(type) == cbor::MajorType::Array;
	}

	template<typename Iterator>
	static bobl::flyweight::lite::utility::Array<T> decode(Iterator& begin, Iterator end)
	{
		auto i = begin;
		bobl::cbor::utility::decode::validate<cbor::MajorType::Array>(begin, end);
		using Decoder = Handler<bobl::flyweight::lite::utility::Any<T>, Options>;
		Decoder::decode_array(begin, end, &Decoder::template decode<Iterator>);
		return { i, begin };
	}
};

template<typename T, typename Options>
class Handler<bobl::flyweight::lite::utility::Object<T>, Options, boost::mpl::true_>
{
public:
	constexpr static bool is(cbor::Type type)
	{
		return bobl::cbor::utility::decode::major_type(type) == cbor::MajorType::Dictionary;
	}

	template<typename Iterator>
	static bobl::flyweight::lite::utility::Object<T> decode(Iterator& begin, Iterator end)
	{
		auto i = begin;
		bobl::cbor::utility::decode::validate<cbor::MajorType::Dictionary>(begin, end);
		using Decoder = Handler<bobl::flyweight::lite::utility::Any<T>, Options>;
		Decoder::decode_array(begin, end, &Decoder::template decode_pair<Iterator>);
		return { i, begin };
	}
};

template<typename Options>
class Handler<boost::uuids::uuid, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static boost::uuids::uuid decode(Iterator& begin, Iterator end)
	{
		bobl::cbor::utility::decode::validate_tag<bobl::cbor::UUID>(begin, end);
		auto data = Handler<std::vector<uint8_t>, bobl::options::None>::decode(begin, end);
		boost::uuids::uuid res;
		if (data.size() != sizeof(res.data))
			throw bobl::InputToShort(str(boost::format("invalid data size of CBOR UUID object (%1 insted of expected %2") % data.size() % sizeof(res.data)));
		std::copy(std::begin(data), std::end(data), res.data);
		return res;
	}
};

template<typename Duration, typename Options>
class Handler<std::chrono::time_point<std::chrono::system_clock, Duration>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static std::chrono::time_point<std::chrono::system_clock, Duration> decode(Iterator& begin, Iterator end)
	{
		auto res = std::chrono::time_point<std::chrono::system_clock, Duration>{};
		bobl::cbor::utility::decode::validate_tag<bobl::cbor::DataTimeNumerical>(begin, end);
		switch (auto type = bobl::cbor::utility::decode::major_type(*begin))
		{
			case bobl::cbor::MajorType::UnsignedInt:
			case bobl::cbor::MajorType::NegativeInt:
				res = bobl::utility::make_timpoint<Duration>(std::chrono::duration<int64_t, typename std::chrono::seconds::period>{bobl::cbor::utility::decode::integer(begin, end)});
				break;
			case bobl::cbor::MajorType::Float:
				res = bobl::utility::make_timpoint<Duration>(std::chrono::duration<double, typename std::chrono::seconds::period>{bobl::cbor::utility::decode::floating_point<double>(begin, end)});
				break;
			default:
				throw bobl::InvalidObject(str(boost::format("CBOR numerical tagged Date/Time has unsupported typr \"%1%\"") % to_string(type)));
		}
		return res;
	}
};

template<typename Options>
class Handler<Name, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Name decode(Iterator& begin, Iterator end) { return Name{ Handler<std::string, bobl::options::None>::decode(begin, end) }; }
};

template<typename T, typename Options>
class Decoder
{
	using Type = typename std::conditional<bobl::utility::Adaptable<T, bobl::cbor::Adapter<T>>::value, bobl::cbor::Adapter<T>, T>::type;
public:
	using type = Handler<Type, typename cbor::EffectiveOptions<T, Options>::type>;
};

template<typename T, typename Options/*, typename Enabled = boost::mpl::true_*/>
class NameValue
{
	NameValue(Name&& name, T&& value) : name_{ std::move(name) }, value_(std::move(value)) {}
public:
	diversion::string_view name() const { return name_(); }
	T const& value() const & { return value_; }
#if !defined(BOOST_GCC_VERSION) || BOOST_GCC_VERSION > 50000 
	//https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60943
	T value() const && { return std::move(value_); }
#endif

	template<typename Iterator, typename NameType>
	static NameValue decode(Iterator& begin, Iterator end, NameType const& ename)
	{
		auto name = Handler<Name, bobl::options::None>::decode(begin, end);
		if(!ename.compare(name()))
			throw bobl::IncorrectObjectName{ str(boost::format("unexpected CBOR object name : \"%1%\" (expected \"%2%\").") % name() % ename.name()) };

		using Decoder = typename Decoder<T, Options>::type;
		return {std::move(name), Decoder::decode(begin, end) };
	}
private:
	Name name_;
	T value_;
};

template<typename T, typename Options/*, typename Enabled = boost::mpl::true_*/>
class NameValue<diversion::optional<T>, Options>
{	
	using Decoder = typename Decoder<T, Options>::type;
	NameValue() : name_{ std::string{} }, value_{ diversion::nullopt }{}
	NameValue(Name&& name, diversion::optional<T>&& value) : name_{ std::move(name) }, value_{ std::move(value) } {}
public:
	diversion::string_view name() const { return {name_()}; }
	diversion::optional<T> const& value() const { return value_; }

	template<typename Iterator, typename NameType>
	static NameValue decode(Iterator& begin, Iterator end, NameType const& ename)
	{
		if (begin == end)
			return { };

		auto tmp = begin;
		auto name = Handler<Name, bobl::options::None>::decode(begin, end);
		return ename.compare(name()) ? NameValue{ std::move(name), decode_null(begin, end) ? diversion::optional<T>{} : diversion::make_optional(Decoder::decode(begin, end)) }
									: (begin = tmp, NameValue{});
	}

	template<typename Iterator>
	static NameValue decode_(Iterator& begin, Iterator end, bobl::utility::ObjectNameIrrelevant const& ename) { return decode_<Decoder, Iterator>(begin, end, ename); }

private:
	template<typename Iterator>
	static bool decode_null(Iterator& begin, Iterator end)
	{
		assert(begin != end);
		boost::ignore_unused(end);
		auto res = cbor::utility::decode::type(*begin) == bobl::cbor::Null;
		if (res)
			++begin;
		return res;
	}

	template<typename Decoder, typename Iterator>
	static auto decode_(Iterator& begin, Iterator end, bobl::utility::ObjectNameIrrelevant const& ename) ->
				typename std::enable_if<!bobl::utility::HasIs<Decoder, bobl::cbor::Type>::value, NameValue>::type
	{
		if (begin != end)
		{
			Iterator tmp = begin;
			auto name = Handler<Name, bobl::options::None>::decode(begin, end);
			try
			{
				if (!is_null(begin, end))
					return { std::move(name), Decoder::decode(begin, end) };
				++begin;
			}
			catch (IncorrectObjectType&)
			{
				begin = tmp;
			}
		}
		return {};
	}
	template<typename Decoder, typename Iterator>
	static auto decode_(Iterator& begin, Iterator end, bobl::utility::ObjectNameIrrelevant const& ename) ->
				typename std::enable_if<bobl::utility::HasIs<Decoder, bobl::cbor::Type>::value, NameValue>::type
	{
		if (begin != end)
		{
			Iterator tmp = begin;
			auto name = Handler<Name, bobl::options::None>::decode(begin, end);
			if (Decoder::is(bobl::cbor::utility::decode::type(*begin)))
				return { std::move(name), Decoder::decode(begin, end) };
			if(is_null(begin, end))
				++begin;
			else
				begin = tmp;
		}
		return {};
	}
private:
	template<typename Iterator>
	static bool is_null(Iterator begin, Iterator end)
	{
		assert(begin != end);
		return *begin == (cbor::SimpleValue | cbor::Null);
	}
private:
	Name name_;
	diversion::optional<T> value_;
};

template<typename T, typename Options>
class NameValue<bobl::flyweight::NameValue<T>, Options> : public NameValue<T, Options>
{
	using Base = NameValue<T, Options>;
	NameValue(Base&& base) : Base{ std::move(base) } {}
public:
	bobl::flyweight::NameValue<T> value() const& { return { std::string(Base::name()), Base::value() }; }
	bobl::flyweight::NameValue<T> value() && { return {std::string(Base::name()), Base::value()}; }
	template<typename Iterator, typename NameType>
	static NameValue decode(Iterator& begin, Iterator end, NameType const& ename) { return { Base::decode(begin, end, ename) };	}
};

template<typename ...Types, typename Options>
class NameValue<diversion::variant<bobl::UseTypeName, Types...>, Options> /*: public NameValue<T, Options>*/
{
	using Value = diversion::variant<bobl::UseTypeName, Types...>;
	NameValue(Value&& value) : value_{ std::move(value) } {}
public:
	Value value() const& { return value_; }
	Value value() && { return std::move(value_); }
	template<typename Iterator>
	static NameValue decode(Iterator& begin, Iterator end)
	{
		return decode_as<0>(Handler<Name, bobl::options::None>::decode(begin, end), begin, end);
	}

	template<typename Iterator, typename NameType>
	static NameValue decode(Iterator& begin, Iterator end, NameType const& /*ename*/) { return decode(begin, end); }
private:
private:
	template<std::size_t N, typename Iterator>
	static auto decode_as(Name const& name, Iterator& begin, Iterator end) -> typename std::enable_if<sizeof...(Types) != N, NameValue>::type
	{
		using Type = typename std::tuple_element<N, std::tuple<Types...>>::type;
		using Decoder = typename Decoder<Type, Options>::type;
		return name().compare(bobl::TypeName<Type>{}()) == 0
								? NameValue{ Decoder::decode(begin, end) }
								: decode_as<N+1>(name, begin, end);
	}

	template<std::size_t N, typename Iterator>
	static auto decode_as(Name const& name, Iterator& /*begin*/, Iterator /*end*/) -> typename std::enable_if<sizeof...(Types) == N, NameValue>::type
	{
		throw bobl::IncorrectObjectName{ str(boost::format("unexpected CBOR object name : \"%1%\".") % name()) };
	}
private:
	Value value_;
};

template<typename ...Types, typename Options>
class Handler<diversion::variant<bobl::UseTypeName, Types...>, Options, boost::mpl::true_>
{
	using Value = diversion::variant<bobl::UseTypeName, Types...>;
public:
	template<typename Iterator>
	static Value decode(Iterator& begin, Iterator end) { return NameValue<Value, Options>::decode(begin, end).value(); }
};

template<typename Iterator, typename ...Options>
struct ArrayValue
{
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = ArrayValue<OtherIterator, OtherOptions...>;

	template<typename T, std::size_t, typename ExpectedName>
	T decode(Iterator& begin, Iterator end, ExpectedName const& /*ename*/) const
	{
		using Decoder = typename Decoder<T, bobl::Options<Options...>>::type;
		return Decoder::decode(begin, end);
	}
};

template<typename Iterator, typename ...Options>
class ObjectDecoder
{
	template<typename T>
	using NameValue = bobl::cbor::decoder::details::NameValue<T, typename cbor::EffectiveOptions<T, Options...>::type>;

	template<typename T>
	struct IsNameValue : std::false_type {};
	template<typename T, typename Opts>
	struct IsNameValue<bobl::cbor::decoder::details::NameValue<T, Opts>> : std::true_type {};
public:
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = ObjectDecoder<OtherIterator, OtherOptions...>;

	template<typename T, std::size_t, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<IsNameValue<T>::value, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{})
	{
		return T::decode(begin, end, ename);
	}

	template<typename T, std::size_t, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<!IsNameValue<T>::value, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{}) const
	{
		return NameValue<T>::decode(begin, end, ename).value();
	}
};

template<typename T, typename ...Options>
class Handler<T, bobl::Options<Options...>, typename boost::mpl::and_<boost::fusion::traits::is_sequence<T>,
													typename boost::mpl::or_<
																	boost::mpl::not_<bobl::utility::options::Contains<typename bobl::cbor::EffectiveOptions<T, Options...>::type, bobl::options::StructAsDictionary>>,
																	boost::mpl::not_<bobl::utility::DictionaryDecoderCompatible<T,
																						typename bobl::cbor::EffectiveOptions<T, Options...>::type>>>::type
																																						>::type>
{
	using HeterogeneousArray = bobl::utility::IsHeterogeneousArraySequence<bobl::cbor::NsTag, T, Options...>;
	template <typename Iterator>
	using ObjectDecoder = typename std::conditional<HeterogeneousArray::value,
												ArrayValue<Iterator, Options...>,
												bobl::cbor::decoder::details::ObjectDecoder<Iterator, Options...>>::type;

	using MajorType = typename std::conditional<HeterogeneousArray::value,
												std::integral_constant<cbor::MajorType, cbor::MajorType::Array>,
												std::integral_constant<cbor::MajorType, cbor::MajorType::Dictionary>>::type;
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		bobl::cbor::utility::decode::validate<MajorType::value>(begin, end);
		auto len = bobl::cbor::utility::decode::lenght(begin, end);
		auto res = bobl::utility::Decoder<T, ObjectDecoder<Iterator>, Options...>{}(begin, end);
		if (len > boost::fusion::result_of::size<T>::value)
		{
			if /*constexpr*/ (bobl::utility::options::Contains<typename bobl::cbor::EffectiveOptions<T, Options...>::type, bobl::options::ExacMatch>::value)
				throw bobl::InvalidObject{ "CBOR dictionary contains more objects than expected" };

			if(len != bobl::cbor::utility::decode::IndefiniteLenght)
				len-=boost::fusion::result_of::size<T>::value;

			using AnyTypeHandler = Handler<bobl::flyweight::lite::Any<Iterator>, typename cbor::EffectiveOptions<T, Options...>::type >;
			AnyTypeHandler::decode_array(len, begin, end, &AnyTypeHandler::decode_pair);
		}
		return res;
	}
};

template<typename Iterator, typename ...Options>
class DictionaryValueDecoder
{
public:
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = DictionaryValueDecoder<OtherIterator, OtherOptions...>;

	using Name = bobl::cbor::decoder::details::Name;
	using Skipper = bobl::flyweight::lite::Any<Iterator>;


	static diversion::string_view name(Name const& name) { return name(); }
	static Name decode_name(Iterator& begin, Iterator end) { return decode<Name>(begin, end); }

	template<typename T>
	static T decode(Name const& /*name*/, Iterator& begin, Iterator end) { return decode<T>(begin, end); }
	template<typename T, typename ExpectedName>
	T decode(Iterator& begin, Iterator end, ExpectedName const& /*ename*/) { return decode<T>(begin, end); }
private:
	template<typename T>
	static T decode(Iterator& begin, Iterator end) 
	{
		using Decoder = typename Decoder<T, typename cbor::EffectiveOptions<T, Options...>::type>::type;
		return Decoder::decode(begin, end); 
	}
};

template<typename T, typename ...Options>
class Handler<T, bobl::Options<Options...>, typename boost::mpl::and_<boost::fusion::traits::is_sequence<T>,
																	bobl::utility::options::Contains<typename bobl::cbor::EffectiveOptions<T, Options...>::type, bobl::options::StructAsDictionary>,
																	bobl::utility::DictionaryDecoderCompatible<T, typename bobl::cbor::EffectiveOptions<T, Options...>::type>>::type>
{
	struct Break
	{
		template<typename Iterator>
		bool operator()(Iterator& begin, Iterator end) const
		{
			if (begin == end)
				throw bobl::InputToShort{ "not enought data to decode CBOR map type" };
			auto val = *begin == bobl::cbor::Break;
			if (val)
				++begin;
			return val;
		}
	};

	class Counter
	{
	public:
		explicit Counter(std::size_t counter) : counter_{ counter } {}
		template<typename Iterator>
		bool operator()(Iterator begin, Iterator end)
		{
			bool res = counter_-- == 0;
			if(!res && begin == end)
				throw bobl::InputToShort{ "not enought data to decode CBOR map type" };
			return res;
		}
	private:
		std::size_t counter_;
	};
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		bobl::cbor::utility::decode::validate<cbor::MajorType::Dictionary>(begin, end);
		auto len = bobl::cbor::utility::decode::lenght(begin, end);
		return  len != bobl::cbor::utility::decode::IndefiniteLenght
					? bobl::utility::DictionaryDecoder<T, bobl::cbor::decoder::details::DictionaryValueDecoder<Iterator>, Options...>{}(begin, end, Counter{ std::size_t(len) })
					: bobl::utility::DictionaryDecoder<T, bobl::cbor::decoder::details::DictionaryValueDecoder<Iterator>, Options...>{}(begin, end, Break{});
	}
};

template<typename T, typename Options>
class Handler<diversion::optional<T>, Options, boost::mpl::true_>
{
	using Decoder = typename Decoder<T, Options>::type;
public:
	constexpr static bool is(cbor::Type type) { return true; }
	template<typename Iterator>
	static diversion::optional<T> decode(Iterator& begin, Iterator end) { return decode_<Decoder, Iterator>(begin, end); }

private:
	template<typename Decoder, typename Iterator>
	static auto decode_(Iterator& begin, Iterator end)
		-> typename std::enable_if<bobl::utility::HasIs<Decoder, bobl::cbor::Type>::value, diversion::optional<T>>::type
	{
		return begin != end && Decoder::is(bobl::cbor::utility::decode::type(*begin))
									? diversion::make_optional(Decoder::decode(begin, end))
									: diversion::nullopt;
	}

	template<typename Decoder, typename Iterator>
	static auto decode_(Iterator& begin, Iterator end)
		-> typename std::enable_if<!bobl::utility::HasIs<Decoder, bobl::cbor::Type>::value, diversion::optional<T>>::type
	{
		if (begin != end)
		{
			auto i = begin;
			try
			{
				return diversion::make_optional(Decoder::decode(begin, end));
			}
			catch (bobl::IncorrectObjectType&)
			{
			}
			begin = i;
		}
		return diversion::nullopt;
	}
};

template<typename ...Params, typename Options>
class Handler<diversion::variant<Params...>, Options, boost::mpl::true_>
{
	static_assert(sizeof...(Params) != 0, "there is no much sense to parse empty variant");
	using ValueType = diversion::variant<Params...>;
public:
	template<typename Iterator>
	static ValueType decode(Iterator& begin, Iterator end)
	{
		if (begin == end)
		{
			auto param_list = bobl::utility::type_name<Params...>();
			throw bobl::InputToShort{ str(boost::format("not enought data to decode CBOR variant <%1%>") % param_list) };
		}
		return try_decode<Iterator, Params...>(begin, end);
	}
private:	
	template<typename Iterator, typename T, typename ...Args >
	static auto try_decode(Iterator& begin, Iterator end) ->  
				typename std::enable_if<bobl::utility::HasIs<typename Decoder<T, Options>::type, bobl::cbor::Type>::value, ValueType>::type
	{
		static_assert(!bobl::utility::IsOptional<T>::value, "there is no much sense in having optional as one of variant values");
		assert(begin != end);
		using Decoder = typename Decoder<T, Options>::type;
		return Decoder::is(bobl::cbor::utility::decode::type(*begin))
				? Decoder::decode(begin, end)
			    : try_decode<Iterator, Args...>(begin, end);
	}

	template<typename Iterator, typename T, typename ...Args >
	static auto try_decode(Iterator& begin, Iterator end) 
		-> typename std::enable_if<!bobl::utility::HasIs<typename Decoder<T, Options>::type, bobl::cbor::Type>::value, ValueType>::type
	{
		auto i = begin;
		try
		{
			using Decoder = typename Decoder<T, Options>::type;
			return { Decoder::decode(begin, end) };
		}
		catch (bobl::IncorrectObjectType&)
		{
		}
		begin = i;
		return try_decode<Iterator, Args...>(begin, end);
	}

	template<typename Iterator>
	static ValueType try_decode(Iterator& begin, Iterator end)
	{
		assert(begin != end);
		auto type = bobl::cbor::utility::decode::type(*begin);
		auto param_list = bobl::utility::type_name<Params...>();
		throw bobl::IncorrectObjectType{ str(boost::format("CBOR value has unexpected type : %1% (%2$#x) insted of expected variant <%3%>") % to_string(type) % int(type) % param_list) };
	}
};

template<typename T, typename Options>
class Handler<bobl::cbor::Adapter<T>, Options, boost::mpl::true_>
{
	using Adaptee = Handler<typename bobl::cbor::Adapter<T>::type, typename cbor::EffectiveOptions<T, Options>::type>;
public:
	template<typename Type>
	static auto is(Type type) -> typename std::enable_if<bobl::utility::HasIs<Adaptee, Type>::value, bool>::type
	{
		static_assert(std::is_same<Type, bobl::cbor::Type>::value, "expected to be called with bobl::cbor::Type");
		return Adaptee::is(type);
	}

	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		auto value = Adaptee::decode(begin, end);
		return bobl::cbor::Adapter<T>{}(std::move(value));
	}
};


} /*namespace details*/} /*namespace decoder*/ }/*namespace cbor*/ } /*namespace bobl*/


