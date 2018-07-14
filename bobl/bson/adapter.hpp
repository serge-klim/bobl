// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/adapter.hpp"
#include <boost/mpl/bool_fwd.hpp>
#include <type_traits>

namespace bobl{ namespace bson { 

template<typename T, typename Enabled = boost::mpl::true_>
class Adapter : public bobl::Adapter<T> {};

} /*namespace bson*/ } /*namespace bobl*/