// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <boost/format.hpp>
#include <boost/type_index.hpp>
#include <string>

namespace bobl{ namespace utility {

template<typename T>
std::string type_name()
{
	return { boost::typeindex::type_id<T>().pretty_name() };
}

template<typename T, typename U, typename ...Args>
std::string type_name()
{
	return str( boost::format("%1%, %2%") % boost::typeindex::type_id<T>().pretty_name() % type_name<U, Args...>());
}


}/*namespace utility*/ } /*namespace bobl*/


