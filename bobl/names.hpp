// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <boost/type_index.hpp>

namespace bobl{ 

template<typename Type>	
class TypeName 
{
public:
	std::string operator()() const { return boost::typeindex::type_id<Type>().pretty_name(); }
};

template<typename Type, typename MemberType, std::size_t Position, typename Options> class MemberName;


} /*namespace bobl*/


