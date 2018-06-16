// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <type_traits>

namespace bobl{ namespace flyweight { 

template<typename T> class NameValue;

namespace utility {

template<typename T> struct IsNameValue : std::false_type {};
template<typename T> struct IsNameValue<NameValue<T>> : std::true_type {};

}/*namespace utility*/}/*namespace flyweight*/ } /*namespace bobl*/


