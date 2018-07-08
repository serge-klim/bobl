// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/details/value.hpp"
#include "bobl/bson/details/header.hpp"
#include "bobl/bson/details/options.hpp"
#include "bobl/bson/adapter.hpp"
#include "bobl/options.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/names.hpp"
#include <boost/mpl/bool_fwd.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/endian/conversion.hpp>
#include <type_traits>
#include <cstdint>

namespace bobl{ namespace bson { namespace flyweight{ 

class Document
{
    Document(std::uint32_t const* size) : size_{ size } {}
public:
    std::size_t size() const { return std::size_t(boost::endian::little_to_native(*size_)); }
	boost::iterator_range<bobl::bson::flyweight::Iterator> value() const
    {
        auto begin = reinterpret_cast<bobl::bson::flyweight::Iterator>(size_ + 1);
		return { begin, begin + size() - sizeof(std::uint32_t) /*document size*/ - 1/*end of document*/ };
    }
    template<typename Iterator>
    static Document decode(Iterator& begin, Iterator end);
private:
    std::uint32_t const* size_;
};

template<typename T, typename Options>
class NameValue 
{
	using Value = typename details::EffectiveValueHandler<T, Options>::type;
	NameValue(Value&& value) : value_{std::move(value)}{}
public:
	diversion::string_view name() const { return value_.name(); }
	T value() const { return value_(); }
	template<typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	static NameValue decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{});
	static NameValue decode(details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end) { return { Value::decode(std::move(header), begin, end) }; }
	template<typename Header>
	static auto is(Header const& header) -> typename std::enable_if<details::HasIs<Value, Header>::value, bool>::type { return Value::is(header); }
private:
	Value value_;
};

template<typename T, typename Options>
class NameValue<diversion::optional<T>, Options>
{
	using Value = typename details::EffectiveValueHandler<T, Options>::type;
	NameValue(diversion::optional<Value>&& value) : value_{ std::move(value) } {}
public:
	diversion::string_view name() const { return !value_ ? diversion::string_view{} : value_.get().name(); }
	diversion::optional<T> value() const { return !value_ ? diversion::nullopt : diversion::make_optional(value_.get()()); }
	template<typename ExpectedName>
	static NameValue decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, ExpectedName const& ename)
	{ 
		if (begin == end)
			return { diversion::nullopt };

		auto header = details::ObjectHeader{ begin, end };
		return !ename.compare(header.name())  
					? NameValue{ diversion::nullopt } 
					: header.type() == bobl::bson::Null 
								? begin =  header.value(), NameValue{ diversion::nullopt }
								: decode(std::move(header), begin, end);
	}


	static NameValue decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, bobl::utility::ObjectNameIrrelevant const& /*ename*/)
	{ 
		return begin == end ? NameValue{ diversion::nullopt } :  try_decode<Value>(begin, end) ;
	}

	static NameValue decode(details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end) { return { diversion::make_optional(Value::decode(std::move(header), begin, end)) }; }

	template<typename Header>
	static auto is(Header const& header) -> typename std::enable_if<details::HasIs<Value, Header>::value, bool>::type { return Value::is(header); }
private:
	template<typename U>
	static NameValue try_decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		auto header = details::ObjectHeader{ begin, end };
		return header.type() == bobl::bson::Null
							? begin = header.value(), NameValue{ diversion::nullopt }
							: NameValue{ try_decode<U>(std::move(header), begin, end) };
	}

	template<typename U>
	static auto try_decode(details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end) -> typename std::enable_if<details::HasIs<U, details::ObjectHeader>::value, diversion::optional<U>>::type
	{
		return U::is(header) ? diversion::make_optional(U::decode(std::move(header), begin, end)) : diversion::nullopt ;
	}

	template<typename U>
	static auto try_decode(details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end) -> typename std::enable_if<!details::HasIs<U, details::ObjectHeader>::value, diversion::optional<U>>::type
	{
		try
		{
			return { U::decode(std::move(header), begin, end) };
		}
		catch (bobl::IncorrectObjectType&)
		{
		}
		begin = header.position();
		return { diversion::nullopt };
	}
private:
	diversion::optional<Value> value_;
};

template<typename T, typename Options>
class NameValue<bobl::flyweight::NameValue<T>, Options> : public NameValue<T, Options>
{
	using Base = NameValue<T, Options>;
	NameValue(Base&& base) : Base{ std::move(base) } {}
public:
	bobl::flyweight::NameValue<T> value() const& { return { std::string(Base::name()), Base::value() }; }
	bobl::flyweight::NameValue<T> value() && { return { std::string(Base::name()), Base::value() }; }
	template<typename NameType>
	static NameValue decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, NameType const& ename) { return { Base::decode(begin, end, ename) }; }
};

template<typename ...Types, typename Options>
class NameValue<diversion::variant<bobl::UseTypeName, Types...>, Options>
{
	using Value = diversion::variant<bobl::UseTypeName, Types...>;
	using TypesTuple = typename boost::mpl::transform<std::tuple<Types...>, details::EffectiveValueHandler<boost::mpl::placeholders::_1, Options>>::type;
	using ValueHandler = typename bobl::utility::MakeVariant<TypesTuple>::type;

	struct ValueVisitor : public boost::static_visitor<Value>
	{
		template<typename T>
		Value operator()(T handler) const { return { handler() }; }
	};
	struct NameVisitor : public boost::static_visitor<diversion::string_view>
	{
		template<typename T>
		diversion::string_view operator()(T handler) const { return handler.name(); }
	};
	NameValue(ValueHandler&& handler) :handler_{ std::move(handler) } {}
public:
	diversion::string_view name() const { return diversion::visit(NameVisitor{}, handler_);}
	Value value() const { return diversion::visit(ValueVisitor{}, handler_); }
	template<typename NameType>
	static NameValue decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, NameType const& /*ename*/);
private:
	template<std::size_t N>
	static auto decode_as(details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end) -> typename std::enable_if<sizeof...(Types) != N, NameValue>::type;
	template<std::size_t N>
	static auto decode_as(details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end) -> typename std::enable_if<sizeof...(Types) == N, NameValue>::type;
private:
	ValueHandler handler_;
};

} /*namespace flyweight*/ 

template<typename ...Args>
auto cast(flyweight::Document const& document) -> typename bobl::utility::DecodeParameters<bobl::bson::NsTag, Args...>::Result;

 }/*namespace bson*/ } /*namespace bobl*/

template<typename Iterator>
bobl::bson::flyweight::Document bobl::bson::flyweight::Document::decode(Iterator& begin, Iterator end)
{
	static_assert(std::is_pointer<Iterator>::value && sizeof(typename std::iterator_traits<Iterator>::value_type) == sizeof(std::uint8_t), "bson Document decode expected pointer to sizeof(std::uint8_t) as iterator");
	auto dataSize = std::distance(begin, end) * sizeof(*begin) / sizeof(std::uint8_t);
	if (sizeof(std::uint32_t) + sizeof(std::uint8_t) > dataSize)
		throw bobl::InvalidObject{ "not enough data provided to construct BSON object" };

	auto document = Document{ reinterpret_cast<std::uint32_t const*>(begin) };
	auto size = document.size();
	if (size != dataSize)
		throw bobl::InvalidObject{ "not enough data provided to construct BSON object" };

	auto b = reinterpret_cast<bobl::bson::flyweight::Iterator>(begin);
	if (b[size - 1] != 0)
		throw bobl::InvalidObject{ "invalid end of object" };
	begin += size;
	return document;
}

template<typename ...Args>
auto bobl::bson::cast(bobl::bson::flyweight::Document const& document) -> typename bobl::utility::DecodeParameters<bobl::bson::NsTag, Args...>::Result
{
	using Parameters = bobl::utility::DecodeParameters<bobl::bson::NsTag, Args...>;
	auto const& value = document.value();
	auto begin = value.begin();
	return Parameters::result(bobl::bson::flyweight::details::SequenceHandler<typename Parameters::Parameters, false, typename Parameters::Options>::decode(begin,value.end()) );
}


template<typename T, typename Options>
template<typename ExpectedName /*= bobl::utility::ObjectNameIrrelevant*/>
bobl::bson::flyweight::NameValue<T, Options> bobl::bson::flyweight::NameValue<T, Options>::decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, ExpectedName const& ename /*= bobl::utility::ObjectNameIrrelevant{}*/)
{
	auto header = bobl::bson::flyweight::details::ObjectHeader{ begin, end };
	if (!ename.compare(header.name()))
		throw bobl::IncorrectObjectName{ str(boost::format("unexpected BSON object name : \"%1%\" (expected \"%2%\").") % header.name() % ename.name()) };
	return decode(std::move(header), begin, end);
}


template<typename ...Types, typename Options>
template<typename NameType /*= bobl::utility::ObjectNameIrrelevant*/>
bobl::bson::flyweight::NameValue<diversion::variant<bobl::UseTypeName, Types...>, Options>
	bobl::bson::flyweight::NameValue<diversion::variant<bobl::UseTypeName, Types...>, Options>::decode(bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end, NameType const& /*ename*/ /*= bobl::utility::ObjectNameIrrelevant{}*/)
{
	auto header = bobl::bson::flyweight::details::ObjectHeader{ begin, end };
	return decode_as<0>(std::move(header), begin, end);
}

template<typename ...Types, typename Options>
template<std::size_t N>
auto bobl::bson::flyweight::NameValue<diversion::variant<bobl::UseTypeName, Types...>, Options>::decode_as(bobl::bson::flyweight::details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
				-> typename std::enable_if<sizeof...(Types) != N, NameValue>::type
{
	using Type = typename std::tuple_element<N, std::tuple<Types...>>::type;
	using Handler = typename details::EffectiveValueHandler<Type, Options>::type;
	return header.name().compare(bobl::TypeName<Type>{}()) == 0
				? NameValue{ ValueHandler{ Handler::decode(std::move(header), begin, end) } }
				: decode_as<N+1>(std::move(header), begin, end);
}

template<typename ...Types, typename Options>
template<std::size_t N>
auto bobl::bson::flyweight::NameValue<diversion::variant<bobl::UseTypeName, Types...>, Options>::decode_as(bobl::bson::flyweight::details::ObjectHeader&& header, bobl::bson::flyweight::Iterator& /*begin*/, bobl::bson::flyweight::Iterator /*end*/)
				-> typename std::enable_if<sizeof...(Types) == N, NameValue>::type
{
	throw bobl::IncorrectObjectName{ str(boost::format("unexpected BSON object name : \"%1%\".") % header.name() ) };
}
