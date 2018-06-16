// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/cbor/details/decoder.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace cbor { 

template<typename To, typename Options, typename Any>
auto cast(Any const& any) -> typename std::enable_if<bobl::flyweight::lite::utility::IsAnyType<Any>::value, To>::type
{ 
	using Decoder = typename bobl::cbor::decoder::details::Decoder<To, Options>::type;
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(any);
	return Decoder::decode(begin, bobl::flyweight::lite::utility::details::end_raw(any));
}

template<typename To, typename Any>
auto cast(Any const& any) -> typename std::enable_if<bobl::flyweight::lite::utility::IsAnyType<Any>::value, To>::type
{ 
	return cast<To, bobl::options::None>(any); 
}

}/*namespace cbor*/ } /*namespace bobl*/


