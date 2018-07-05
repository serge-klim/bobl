// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/cbor/options.hpp"

namespace bobl{ 
		
namespace cbor { struct NsTag {}; }/*namespace cbor*/

template<typename ...Args>
struct EffectiveOptions<bobl::cbor::NsTag, Args...> : cbor::EffectiveOptions<Args...> {};

}/*namespace bobl*/
