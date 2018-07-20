// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/cbor/details/iterator.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace cbor { 

template<typename T, typename Options, typename Object>
auto make_iterator_range(Object const& object)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsObject<Object>::value, boost::iterator_range<details::ObjectIterator<Object, T, Options>>>::type
{
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(object);
	auto end = bobl::flyweight::lite::utility::details::end_raw(object);
	if(begin != end)
		utility::decode::length(begin, end);
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
	if (begin != end)
		utility::decode::length(begin, end);
	return boost::make_iterator_range(details::ArrayIterator<Array, T, Options>{begin, end}, details::ArrayIterator<Array, T, Options>{end, end});
}

template<typename T, typename Array>
auto make_iterator_range(Array const& array)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsArray<Array>::value, boost::iterator_range<details::ArrayIterator<Array, T, bobl::options::None>>>::type
{
	return make_iterator_range<T, bobl::options::None, Array>(array);
}

template<typename T, typename Options, typename RawIterator>
boost::iterator_range<bobl::utility::Iterator<T, Options, details::ValueDecoder, RawIterator>> make_iterator_range(RawIterator begin, RawIterator end)
{
	return boost::make_iterator_range(bobl::utility::Iterator<T, Options, details::ValueDecoder, RawIterator>{begin, end},
										bobl::utility::Iterator<T, Options, details::ValueDecoder, RawIterator>{end, end});
}


template<typename T, typename RawIterator>
boost::iterator_range<bobl::utility::Iterator<T, bobl::options::None, details::ValueDecoder, RawIterator>> make_iterator_range(RawIterator begin, RawIterator end)
{
	return make_iterator_range<T, bobl::options::None, RawIterator>(begin, end);
}


}/*namespace cbor*/ } /*namespace bobl*/


