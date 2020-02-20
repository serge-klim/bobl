// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/details/header.hpp"
#include "bobl/bson/details/options.hpp"
#include "bobl/bson/adapter.hpp"
#include "bobl/bson/bson.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/nvariant.hpp"
#include "bobl/utility/timepoint.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/float.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/names.hpp"
#include "bobl/bobl.hpp"
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/format.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <cstdint>
#include <cassert>


namespace bobl{ namespace bson { namespace encoder{ namespace details { 

template<typename T, typename Options, typename Enabled = boost::mpl::true_>
class Handler{};

using Header = std::pair<bobl::bson::Type,diversion::string_view>;

template<typename Options>
class Handler<Header, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, Header const& header)
	{
		out = std::uint8_t(header.first);
		std::copy(std::begin(header.second), std::end(header.second), ++out);
		out = 0;
		return ++out;
	}
};

template<typename Options>
class Handler<bool, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, bool value)
	{
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::Bool, std::move(name)));
		out = boost::endian::native_to_little(std::uint8_t(value));
		return ++out;
	}
};

template<typename Iterator, typename T>
Iterator encode_integer(Iterator out, T value)
{
	static_assert(std::is_integral<T>::value, "value expected to be an integer");
	boost::endian::little_to_native_inplace(value);
	auto begin = reinterpret_cast<flyweight::Iterator>(&value);
	auto end = begin + sizeof(value) / sizeof(std::uint8_t);
	return std::copy(begin, end, out);
}

template</*typename T,*/ typename Options>
class Handler<std::string, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, std::string const& value)
	{
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::Utf8String, std::move(name)));
		out = encode_integer(out, std::uint32_t(value.size() + 1));  // size 
		out = std::copy(std::begin(value), std::end(value), out);	 // value
		out = 0;
		return ++out;
	}
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, typename bobl::utility::IsByteType<T, Options>::type>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, std::vector<T> const& value)
	{
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::Binary, std::move(name)));
		out = encode_integer(out, std::uint32_t(value.size()));			// size 
		out = encode_integer(out, std::uint8_t{ bobl::bson::Generic});	// subtype 
		return std::copy(std::begin(value), std::end(value), out);		// value
	}
};

template<typename Options>
class Handler<boost::uuids::uuid, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, boost::uuids::uuid const& value)
	{
		auto size = sizeof(value.data);
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::Binary, std::move(name)));
		out = encode_integer(out, std::uint32_t(size));				 // size 
		out = encode_integer(out, std::uint8_t{ bobl::bson::Uuid }); // subtype 
		return std::copy(value.data, value.data + size, out);		 // value
	}
};


template<typename Duration, typename Options>
class Handler<std::chrono::time_point<std::chrono::system_clock, Duration>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> const& value)
	{
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::UTCDateTime, std::move(name)));
		auto duration = bobl::utility::duration_since_epoch<std::chrono::duration<int64_t, typename std::chrono::milliseconds::period>>(value);
		return encode_integer(out, duration.count());
	}
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_integral<T>::value>>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, T val)
	{
		return encode_<typename bobl::utility::options::Contains<typename bobl::bson::EffectiveOptions<T, Options>::type, bobl::options::IntegerOptimizeSize>::type>(std::move(out), std::move(name), val);
	}
private:
	template<typename IntegerOptimizeSize, typename Iterator, typename U>
	static typename std::enable_if<IntegerOptimizeSize::value, Iterator>::type encode_(Iterator out, diversion::string_view name, U val)
	{
		using Int32type = typename std::conditional<std::is_signed<U>::value, std::int32_t, std::uint32_t>::type;
		return val > std::numeric_limits<std::uint32_t>::max() || val < std::numeric_limits<std::uint32_t>::min() 
						? encode_<boost::mpl::false_>(std::move(out), std::move(name), val)
						: encode_<boost::mpl::false_>(std::move(out), std::move(name), Int32type(val));
	}

	template<typename IntegerOptimizeSize, typename Iterator, typename U>
	static typename std::enable_if<!IntegerOptimizeSize::value, Iterator>::type encode_(Iterator out, diversion::string_view name, U val)
	{
		using Types = typename std::conditional<(sizeof(U) > sizeof(std::uint32_t)), 
									std::pair<std::integral_constant<bobl::bson::Type, bobl::bson::Type::Int64>, 
												typename std::conditional<std::is_signed<U>::value, std::int64_t, std::uint64_t>::type>,
									std::pair<std::integral_constant<bobl::bson::Type, bobl::bson::Type::Int32 >,
												typename std::conditional<std::is_signed<U>::value, std::int32_t, std::uint32_t>::type>
											>::type;

		out = Handler<Header, Options>::encode(out, std::make_pair(Types::first_type::value, std::move(name)));
		return encode_integer(out, typename Types::second_type(val));
	}
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_floating_point<T>::value>>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, T val)
	{
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::Double, std::move(name)));
		auto value = boost::endian::native_to_little(bobl::utility::FloatConverter<sizeof(std::uint64_t)>{}(val));
		auto begin = reinterpret_cast<flyweight::Iterator>(&value);
		auto end = begin + sizeof(value) / sizeof(std::uint8_t);
		return std::copy(begin, end, out);
	}
};


template<typename Options, typename Iterator, typename T>
inline Iterator encode(Iterator out, diversion::string_view name, T const& value)
{
	using Type = typename std::conditional<bobl::utility::Adaptable<T, bobl::bson::Adapter<T>>::value, bobl::bson::Adapter<T>, T>::type;
	return Handler<Type, typename bobl::bson::EffectiveOptions<T, Options>::type>::encode(std::move(out), std::move(name), value);
}

template<typename T, typename Options>
class Handler<diversion::optional<T>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, diversion::optional<T> const& value)
	{
		return !value
			? encode_empty<Options>(std::move(out), std::move(name))
			: details::encode<Options, Iterator, T>(std::move(out), std::move(name), value.get());
	}
private:
	template<typename Opts, typename Iterator>
	static auto encode_empty(Iterator out, diversion::string_view /*name*/)
		-> typename std::enable_if<!bobl::utility::options::Contains<Opts, bobl::options::OptionalAsNull>::value, Iterator>::type
	{
		return out;
	}

	template<typename Opts, typename Iterator>
	static auto encode_empty(Iterator out, diversion::string_view name)
		-> typename std::enable_if<bobl::utility::options::Contains<Opts, bobl::options::OptionalAsNull>::value, Iterator>::type
	{
		return Handler<Header, Options>::encode(std::move(out), std::make_pair(bobl::bson::Null, std::move(name)));
	}
};

template<typename ...Types, typename Options>
class Handler<diversion::variant<Types...>, Options, typename boost::mpl::not_<typename bobl::utility::VariantUseTypeName<diversion::variant<Types...>,
																				typename bobl::bson::EffectiveOptions<diversion::variant<Types...>, Options>::type>>::type>
{
	template<typename Iterator>
	struct ValueVisitor : public boost::static_visitor<Iterator>
	{
		ValueVisitor(Iterator out, diversion::string_view name) : out{ std::move(out) }, name{ std::move(name) } {}
		template<typename T>
		Iterator operator()(T const& value) const 
		{ 
			return details::encode<Options, Iterator, T>(std::move(out), std::move(name), value);
		}
	private:
		Iterator out;
		diversion::string_view name;
	};
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, diversion::variant<Types...> const& value)
	{
		return diversion::visit(ValueVisitor<Iterator>{ std::move(out), std::move(name) }, value);
	}
};

template<typename ...Types, typename Options>
class Handler<diversion::variant<Types...>, Options, typename bobl::utility::VariantUseTypeName<diversion::variant<Types...>,
															typename bobl::bson::EffectiveOptions<diversion::variant<Types...>, Options>::type>::type>
{
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
			auto name = bobl::TypeName<T>{}();
			return details::encode<Options, Iterator, T>(std::move(out_), diversion::string_view{ name }, value);
		}
	private:
		Iterator out_;
	};
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::variant<Types...> const& value)
	{
		return diversion::visit(ValueVisitor<Iterator>{ std::move(out) }, value);
	}
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view /*name*/, diversion::variant<Types...> const& value)
	{
		return encode(std::move(out), value);
	}
};

template<typename T, typename Options>
class Handler<bobl::bson::Adapter<T>, Options, boost::mpl::true_>
{
	using Adapter = bobl::bson::Adapter<T>;
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, T const& value)
	{
		return Handler<typename Adapter::type, typename bson::EffectiveOptions<T, Options>::type>::encode(std::move(out), std::move(name), Adapter{}(value));
	}
};


template<typename T,typename Options>
class Handler<T, Options, typename boost::fusion::traits::is_sequence<T>::type>
{
	using HeterogeneousArray = bobl::utility::IsHeterogeneousArraySequence<bobl::bson::NsTag, T, Options>;
	static_assert(boost::mpl::or_<HeterogeneousArray, bobl::utility::NamedSequence<T, typename bobl::bson::EffectiveOptions<T, Options>::type>>::value ,
									"can't encode unnamed sequence as BSON object");
public:
	static void encode(std::vector<std::uint8_t>& buffer, T const& sequence)
	{
		auto first = buffer.size();
		buffer.resize(first + sizeof(std::uint32_t) / sizeof(std::uint8_t));
		encode<0>(std::back_inserter(buffer), sequence);
		buffer.push_back(0);
		encode_integer(std::next(std::begin(buffer), first), std::uint32_t(buffer.size() - first));		
	}

	template<typename Iterator>
	static Iterator encode(Iterator out, T const& sequence)
	{
		std::vector<std::uint8_t> buffer;
		encode(buffer, sequence);
		return std::copy(std::begin(buffer), std::end(buffer), out);
	}

	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, T const& sequence)
	{
		using BsonType = typename std::conditional<HeterogeneousArray::value, std::integral_constant<bobl::bson::Type, bobl::bson::Array>, std::integral_constant<bobl::bson::Type, bobl::bson::EmbeddedDocument>>::type;
		Handler<Header, Options>::encode(out, std::make_pair(BsonType::value, std::move(name)));
		return encode(out, sequence);
	}
private:
	template<std::size_t N, typename U>
	static auto name() -> typename std::enable_if<HeterogeneousArray::value, decltype(diversion::to_string(N))>::type
	{
		return diversion::to_string(N);
	}

	template<std::size_t N, typename U>
	static auto name()
		-> typename std::enable_if<!HeterogeneousArray::value, decltype(bobl::utility::GetNameType<bobl::MemberName<T, U, N, typename bobl::bson::EffectiveOptions<T, Options>::type>>::type::name())>::type
	{
		using MemberName = typename bobl::utility::GetNameType<bobl::MemberName<T, U, N, typename bobl::bson::EffectiveOptions<T, Options>::type>>::type;
		return MemberName::name();
	}

	template<std::size_t N, typename U, typename Iterator>
	static typename std::enable_if<boost::fusion::result_of::size<U>::value != N, Iterator>::type encode(Iterator out, U const& sequence)
	{
		using Type = typename boost::fusion::result_of::value_at_c<U, N>::type;
		auto const& n = name<N, Type>();
		out = details::encode<Options, Iterator, Type>(out, diversion::string_view{ n }, boost::fusion::at_c<N>(sequence));
		return encode<N + 1>(std::move(out), sequence);
	}

	template<std::size_t N, typename U, typename Iterator>
	static typename std::enable_if<boost::fusion::result_of::size<U>::value == N, Iterator>::type encode(Iterator out, U const& /*sequence*/) { return out; }
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, typename boost::mpl::not_<bobl::utility::IsByteType<T, Options>>::type>
{
public:
	template<typename Iterator>
	static Iterator encode(Iterator out, diversion::string_view name, std::vector<T> const& values)
	{
		Handler<Header, Options>::encode(out, std::make_pair(bobl::bson::Array, std::move(name)));
		std::vector<std::uint8_t> buffer;
		auto bi = std::back_inserter(buffer);
		auto n = values.size();
		for (decltype(n) i = 0; i != n; ++i)
		{
			auto item_name = diversion::to_string(i);
			bi = details::encode<Options, Iterator, T>(bi, diversion::string_view{ item_name }, values[i]);
		}
		out = encode_integer(out, std::uint32_t(buffer.size() + sizeof(std::uint32_t) / sizeof(std::uint8_t) + 1));
		out = std::copy(std::begin(buffer), std::end(buffer), out);
		out = 0;
		return ++out;
	}
};


} /*namespace details*/ } /*namespace encoder*/ } /*namespace bson*/ } /*namespace bobl*/

