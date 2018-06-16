// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <type_traits>

namespace bobl{ namespace utility {

template<typename T, typename Param, typename U = bool>
struct HasIs : std::false_type {};

template<typename T, typename Param>
struct HasIs<T, Param, decltype(T::is(std::declval<Param>()))> : std::true_type {};

}/*namespace utility*/ } /*namespace bobl*/


