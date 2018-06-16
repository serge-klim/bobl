// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <utility>
#include <type_traits>


namespace bobl{ namespace utility { 
	
template<typename T, typename Adapter, typename Enabled = std::true_type>
struct Adaptable : std::false_type {};

template<typename T, typename Adapter>
struct Adaptable<T, Adapter, typename std::is_same<typename Adapter::type, decltype(std::declval<Adapter>()(std::declval<T>()))>::type> : std::true_type {};


}/*namespace utility*/ } /*namespace bobl*/


