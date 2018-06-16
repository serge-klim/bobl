// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/diversion.hpp"
#include <tuple>
#include <type_traits>

namespace bobl{ namespace utility {

template <typename ...Params> 
struct MakeVariant
{
	using type = diversion::variant<Params...>;
};

template <typename ...Params>
struct MakeVariant<std::tuple<Params...>>
{
	using type = diversion::variant<Params...>;
};

template<typename T> struct IsOptional : std::false_type {};
template<typename T> struct IsOptional<diversion::optional<T>> : std::true_type {};

}/*namespace utility*/ } /*namespace bobl*/

