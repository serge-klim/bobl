// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <boost/mpl/bool_fwd.hpp>

namespace bobl{ 

template<typename T, typename Enabled = boost::mpl::true_>
class Adapter {};

template<typename T>
struct Adapter<T, boost::mpl::bool_<std::is_enum<T>::value>>
{
	using type = typename std::underlying_type<T>::type;
	T operator()(type x) const { return T(x); }
	type operator()(T const& x) const { return type(x); }
};

} /*namespace bobl*/


