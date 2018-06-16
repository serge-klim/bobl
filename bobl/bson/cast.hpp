// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/bson/details/value.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace bson { 

template<typename To, typename Options, typename Any>
auto cast(Any const& any) -> typename std::enable_if<bobl::flyweight::lite::utility::IsAnyType<Any>::value, To>::type
{ 
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(any);
	return bobl::bson::flyweight::NameValue<To, Options>::decode(begin, bobl::flyweight::lite::utility::details::end_raw(any), bobl::utility::ObjectNameIrrelevant{}).value();
}

template<typename To, typename Any>
auto cast(Any const& any) -> typename std::enable_if<bobl::flyweight::lite::utility::IsAnyType<Any>::value, To>::type
{ 
	return cast<To, bobl::options::None>(any); 
}

}/*namespace bson*/ } /*namespace bobl*/


