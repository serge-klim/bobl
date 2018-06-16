// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/adapter.hpp"
#include <type_traits>

namespace bobl{ namespace cbor { 

template<typename T, typename Enabled = std::true_type>
class Adapter : public bobl::Adapter<T> {};

}/*namespace cbor*/ } /*namespace bobl*/