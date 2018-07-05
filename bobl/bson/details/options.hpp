// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/options.hpp"

namespace bobl{ 
		
namespace bson { struct NsTag {}; }/*namespace bson*/

template<typename ...Args>
struct EffectiveOptions<bobl::bson::NsTag, Args...> : bson::EffectiveOptions<Args...> {};

}/*namespace bobl*/
