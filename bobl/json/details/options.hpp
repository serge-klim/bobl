// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/json/options.hpp"

namespace bobl{ 
		
namespace json { struct NsTag {}; }/*namespace json*/

template<typename ...Args>
struct EffectiveOptions<bobl::json::NsTag, Args...> : json::EffectiveOptions<Args...> {};

}/*namespace bobl*/
