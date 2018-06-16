// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/options.hpp"
namespace bobl{ namespace json {

template<typename T, typename ...Options>
struct EffectiveOptions : bobl::EffectiveOptions<T, Options...> {};

template<typename T, typename ...Options>
struct EffectiveOptions<T, bobl::Options<Options...>> : EffectiveOptions<T, Options...> {};

}/*namespace json*/ } /*namespace bobl*/
