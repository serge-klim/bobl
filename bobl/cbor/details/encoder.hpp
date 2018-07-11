// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/cbor/details/utility.hpp"
#include "bobl/cbor/details/options.hpp"
#include "bobl/cbor/adapter.hpp"
#include "bobl/cbor/cbor.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/timepoint.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/float.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/bobl.hpp"
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/format.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <cstdint>
#include <cassert>


namespace bobl{ namespace cbor { namespace encoder{ namespace details { 

template<typename T, typename Options, typename Enabled = boost::mpl::true_>
class Handler{};

template<typename Options>
class Handler<bool, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, bool value)
	{
		out = value ? bobl::cbor::True : bobl::cbor::False;
		return ++out;
	}
};

template</*typename T,*/ typename Options>
class Handler<diversion::string_view, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view const& value)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::TextString, value.length());
		return std::copy(std::begin(value), std::end(value), out);
	}
};

template</*typename T,*/ typename Options>
class Handler<std::string, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, std::string const& value)
	{
		return Handler<diversion::string_view, Options>::encode(out, diversion::string_view{ value });
	}
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, typename bobl::utility::IsByteType<T, Options>::type>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, std::vector<T> const& value)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::ByteString, value.size());
		return std::copy(std::begin(value), std::end(value), out);
	}
};


template<typename Options>
class Handler<boost::uuids::uuid, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, boost::uuids::uuid const& value)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::Tag, std::uint8_t(bobl::cbor::UUID));
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::ByteString, value.size());
		return std::copy(value.data, value.data + sizeof(value.data), out);
	}
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_integral<T>::value>>
{
public:
	template<typename Iterator>
	static typename std::enable_if<std::is_signed<T>::value, Iterator>::type encode(Iterator out, T value)
	{
		return value < 0
				? encode_<typename bobl::utility::options::Contains<typename bobl::cbor::EffectiveOptions<T, Options>::type, bobl::options::IntegerOptimizeSize>::type>
							(std::move(out), bobl::cbor::MajorType::NegativeInt, typename std::make_unsigned<T>::type(typename std::make_unsigned<T>::type(std::abs(value)) - 1))
				: encode_<typename bobl::utility::options::Contains<typename bobl::cbor::EffectiveOptions<T, Options>::type, bobl::options::IntegerOptimizeSize>::type>
							(std::move(out), bobl::cbor::MajorType::UnsignedInt, typename std::make_unsigned<T>::type(value));
	}
	template<typename Iterator>
	static typename std::enable_if<!std::is_signed<T>::value, Iterator>::type encode(Iterator out, T value)
	{
		return encode_<typename bobl::utility::options::Contains<typename bobl::cbor::EffectiveOptions<T, Options>::type, bobl::options::IntegerOptimizeSize>::type>(std::move(out), bobl::cbor::MajorType::UnsignedInt, value);
	}
private:
	template<typename IntegerOptimizeSize, typename Iterator, typename U>
	static typename std::enable_if<IntegerOptimizeSize::value, Iterator>::type encode_(Iterator out, bobl::cbor::MajorType type, U value)
	{
		static_assert(std::is_integral<U>::value && !std::is_signed<U>::value, "U expected to be unsigned integral type");
		return bobl::cbor::utility::encode::unsigned_int(std::move(out), type, value);
	}

	template<typename IntegerOptimizeSize, typename Iterator, typename U>
	static typename std::enable_if<!IntegerOptimizeSize::value, Iterator>::type encode_(Iterator out, bobl::cbor::MajorType type, U value)
	{
		static_assert(std::is_integral<U>::value && !std::is_signed<U>::value, "U expected to be unsigned integral type");
		return bobl::cbor::utility::encode::unsigned_int_strict(out, type, value);
	}	
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_floating_point<T>::value>>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, T const& value)
	{		
		auto i = bobl::utility::FloatConverter<sizeof(value)>{}(value);
		return i<=(std::numeric_limits<std::uint8_t>::max)()
				? bobl::cbor::utility::encode::unsigned_int_strict(out, bobl::cbor::MajorType::Float, std::uint16_t(i))
				: bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::Float, i);
	}
};

template<typename Duration, typename Options>
class Handler<std::chrono::time_point<std::chrono::system_clock, Duration>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, std::chrono::time_point<std::chrono::system_clock, Duration> const& value)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::Tag, std::uint8_t(bobl::cbor::DataTimeNumerical));
		auto duration = bobl::utility::duration_since_epoch<std::chrono::duration<int64_t, typename std::chrono::seconds::period>>(value);
		return Handler<std::int64_t, bobl::options::None>::encode(out, duration.count());
	}
};

template<typename Options, typename Iterator, typename T>
inline Iterator encode(Iterator out, T const& value)
{
	using Type = typename std::conditional<bobl::utility::Adaptable<T, bobl::cbor::Adapter<T>>::value, bobl::cbor::Adapter<T>, T>::type;
	return Handler<Type, typename bobl::cbor::EffectiveOptions<T, Options>::type>::encode(std::move(out), value);
}

template<typename T, typename Options>
class Handler<T, Options, typename boost::mpl::and_<boost::fusion::traits::is_sequence<T>, bobl::utility::IsHeterogeneousArraySequence<bobl::cbor::NsTag, T, Options>>::type>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, T const& sequence)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::Array, std::size_t(boost::fusion::result_of::size<T>::value));
		return encode<0>(out, sequence);
	}
private:
	template<std::size_t N, typename U, typename Iterator>
	static typename std::enable_if<boost::fusion::result_of::size<U>::value != N, Iterator>::type encode(Iterator out, U const& sequence)
	{
		using Type = typename boost::fusion::result_of::value_at_c<U, N>::type;
		out = details::encode<Options, Iterator, Type>(out, boost::fusion::at_c<N>(sequence));
		return encode<N + 1>(std::move(out), sequence);
	}

	template<std::size_t N, typename U, typename Iterator>
	static typename std::enable_if<boost::fusion::result_of::size<U>::value == N, Iterator>::type encode(Iterator out, U const& /*sequence*/) { return out; }
};

template<typename T, typename Options>
class Handler<T, Options, typename boost::mpl::and_<boost::fusion::traits::is_sequence<T>,
														bobl::utility::IsObjectSequence<bobl::cbor::NsTag, T, Options>, 
														bobl::utility::NamedSequence<T, typename bobl::cbor::EffectiveOptions<T, Options>::type>>::type>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, T const& sequence)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::Dictionary, std::size_t(boost::fusion::result_of::size<T>::value));
		return encode<0>(out, sequence);
	}
private:
	template<std::size_t N, typename U, typename Iterator>
	static typename std::enable_if<boost::fusion::result_of::size<U>::value != N, Iterator>::type encode(Iterator out, U const& sequence)
	{
		using Type = typename boost::fusion::result_of::value_at_c<U, N>::type;
		using MemberName = typename bobl::utility::GetNameType<bobl::MemberName<U, Type, N, Options>>::type;
		out = encode(out, MemberName::name(), boost::fusion::at_c<N>(sequence));
		return encode<N + 1>(std::move(out), sequence);
	}

	template<std::size_t N, typename U, typename Iterator>
	static typename std::enable_if<boost::fusion::result_of::size<U>::value == N, Iterator>::type encode(Iterator out, U const& /*sequence*/) { return out; }

	template<typename U, typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, U const& value)
	{
		out = Handler<diversion::string_view, bobl::options::None>::encode(out, name);
		return details::encode<Options, Iterator, U>(out, value);
	}

	template<typename U, typename Iterator>
	static auto encode(Iterator out, diversion::string_view name, diversion::optional<U> const& value)
		-> typename std::enable_if<!bobl::utility::Adaptable<diversion::optional<U>, bobl::cbor::Adapter<diversion::optional<U>>>::value, Iterator>::type
	{
		return Handler<diversion::optional<U>, typename bobl::cbor::EffectiveOptions<diversion::optional<U>, Options>::type>::encode(out, name, value);
	}

	template<typename ...Types, typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view /*name*/, diversion::variant<bobl::UseTypeName, Types...> const& value)
	{
		return details::encode<Options, Iterator, diversion::variant<bobl::UseTypeName, Types...>>(out, value);
	}
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, typename boost::mpl::not_<bobl::utility::IsByteType<T, Options>>::type>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, std::vector<T> const& value)
	{
		out = bobl::cbor::utility::encode::unsigned_int(out, bobl::cbor::MajorType::Array, value.size());
		for(auto const& i : value)
			out = details::encode<Options, Iterator, T>(out, i);
		return out;
	}
};

template<typename T, typename Options>
class Handler<diversion::optional<T>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, diversion::optional<T> const& value)
	{
		if (!value == false || bobl::utility::options::Contains<Options, bobl::options::OptionalAsNull>::value)
		{
			out = Handler<diversion::string_view, bobl::options::None>::encode(std::move(out), name);
			out = encode(std::move(out), value);
		}
		return out;
	}

	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::optional<T> const& value)
	{
		return !value
			? encode_empty<Options>(std::move(out))
			: details::encode<Options, Iterator, T>(std::move(out), value.get());
	}
private:
	template<typename Opts, typename Iterator>
	static auto encode_empty(Iterator out) ->
		typename std::enable_if<!bobl::utility::options::Contains<Opts, bobl::options::OptionalAsNull>::value, Iterator>::type
	{
		return out;
	}

	template<typename Opts, typename Iterator>
	static auto encode_empty(Iterator out) ->
		typename std::enable_if<bobl::utility::options::Contains<Opts, bobl::options::OptionalAsNull>::value, Iterator>::type
	{
		out = std::uint8_t(bobl::cbor::Null);
		return ++out;
	}
};

template<typename ...Types, typename Options>
class Handler<diversion::variant<Types...>, Options, boost::mpl::true_>
{
	template<typename Iterator>
	struct ValueVisitor : public boost::static_visitor<Iterator>
	{
		ValueVisitor(Iterator out) : out{ std::move(out) } {}
		template<typename T>
		Iterator operator()(T const& value) const
		{
			return details::encode<Options, Iterator, T>(std::move(out), value);
		}
	private:
		Iterator out;
	};
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::variant<Types...> const& value)
	{
		return diversion::visit(ValueVisitor<Iterator>{ std::move(out) }, value);
	}
};

template<typename ...Types, typename Options>
class Handler<diversion::variant<bobl::UseTypeName, Types...>, Options, boost::mpl::true_>
{
	static_assert(sizeof...(Types)!=0, "variant<bobl::UseTypeName> doesn't make much sense");
	template<typename Iterator>
	struct ValueVisitor : public boost::static_visitor<Iterator>
	{
		ValueVisitor(Iterator out) : out_{ std::move(out) } {}
		Iterator operator()(bobl::UseTypeName const&) const 
		{ 
			auto param_list = bobl::utility::type_name<Types...>();
			throw bobl::InvalidObject{ str(boost::format("can't encode variant<bobl::UseTypeName, %1%> with no value in it") % param_list) };
		}
		template<typename T>
		Iterator operator()(T const& value) const
		{
			std::string const& name = bobl::TypeName<decltype(value)>{}();
			Iterator out = details::encode<bobl::options::None, Iterator, std::string>(std::move(out_), name);
			return details::encode<Options, Iterator, T>(std::move(out), value);
		}
	private:
		Iterator out_;
	};
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::variant<bobl::UseTypeName, Types...> const& value)
	{
		return diversion::visit(ValueVisitor<Iterator>{ std::move(out) }, value);
	}
};

template<typename T, typename Options>
class Handler<bobl::cbor::Adapter<T>, Options, boost::mpl::true_>
{
	using Adapter = bobl::cbor::Adapter<T>;
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, T const& value)
	{
		return Handler<typename Adapter::type, typename cbor::EffectiveOptions<T, Options>::type>::encode(std::move(out), Adapter{}(value));
	}
};

template<typename Options, typename Iterator, typename T, typename ...Args>
Iterator encode(Iterator out, T const& value, Args&& ...args)
{
	details::Handler<T, Options>::encode(out, value);
	return encoder::details::encode<bobl::Options<Options>>(std::move(out), std::forward<Args>(args)...);
}

template<typename Options, typename Iterator> Iterator encode(Iterator out) { return out; }

} /*namespace details*/ } /*namespace encoder*/ } /*namespace cbor*/ } /*namespace bobl*/

