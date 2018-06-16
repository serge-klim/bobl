// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/json/details/iterator.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace json { 

template <typename T> 
struct Traits
{
};

template<typename RawIterator>
struct Traits<bobl::flyweight::lite::Array<RawIterator>>
{
	template<typename U, typename Options = bobl::options::None>
	using Iterator = bobl::utility::Iterator<U, Options, details::ValueDecoder, RawIterator>;
};

template<typename RawIterator>
struct Traits<bobl::flyweight::lite::Object<RawIterator>>
{
	template<typename U, typename Options = bobl::options::None>
	using Iterator = bobl::utility::Iterator<U, Options, details::NameValueDecoder, RawIterator>;
};

template<typename T, typename Options, typename Object>
auto make_iterator_range(Object const& object)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsObject<Object>::value, boost::iterator_range<details::ObjectIterator<Object, T, Options>>>::type
{
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(object);
	auto end = bobl::flyweight::lite::utility::details::end_raw(object);
	parser::ObjectOpen::parse(begin, end);
	return boost::make_iterator_range(details::ObjectIterator<Object, T, Options>{begin, end}, details::ObjectIterator<Object, T, Options>{end, end});
}

template<typename T, typename Object>
auto make_iterator_range(Object const& object)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsObject<Object>::value, boost::iterator_range<details::ObjectIterator<Object, T, bobl::options::None>>>::type
{
	return make_iterator_range<T, bobl::options::None, Object>(object);
}

template<typename T, typename Options, typename Array>
auto make_iterator_range(Array const& array)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsArray<Array>::value, boost::iterator_range<details::ArrayIterator<Array, T, Options>>>::type
{
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(array);
	auto end = bobl::flyweight::lite::utility::details::end_raw(array);
	parser::ArrayOpen::parse(begin, end);
	return boost::make_iterator_range(details::ArrayIterator<Array, T, Options>{begin, end}, details::ArrayIterator<Array, T, Options>{end, end});
}

template<typename T, typename Array>
auto make_iterator_range(Array const& array)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsArray<Array>::value, boost::iterator_range<details::ArrayIterator<Array, T, bobl::options::None>>>::type
{
	return make_iterator_range<T, bobl::options::None, Array>(array);
}

}/*namespace json*/ } /*namespace bobl*/


