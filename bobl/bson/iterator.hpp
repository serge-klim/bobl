// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/bson/details/iterator.hpp"
#include "bobl/bson/flyweight.hpp"
#include "bobl/utility/iterator.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace bson { 


template<typename T, typename Options = bobl::options::None>
using DocumentIterator = typename details::Traits<flyweight::Document>::template Iterator<T, Options>;

template<typename T, typename Options = bobl::options::None>
boost::iterator_range<DocumentIterator<T, Options>> make_iterator_range(flyweight::Document const& doc)
{
	auto value = doc.value();
	auto begin = value.begin();
	auto end = value.end();
	return boost::make_iterator_range(DocumentIterator<T, Options>{begin, end}, DocumentIterator<T, Options >{end, end});
}

template<typename T, typename Options, typename Object>
auto make_iterator_range(Object const& object)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsObject<Object>::value, boost::iterator_range<details::ObjectIterator<Object, T, Options>>>::type
{
	auto raw_begin = bobl::flyweight::lite::utility::details::begin_raw(object);
	auto raw_end = bobl::flyweight::lite::utility::details::end_raw(object);
	auto doc = flyweight::details::EmbeddedDocument::decode_as<bobl::bson::EmbeddedDocument>(raw_begin, raw_end);
	auto value = doc.value();
	auto begin = value.begin();
	auto end = value.end();
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
	auto raw_begin = bobl::flyweight::lite::utility::details::begin_raw(array);
	auto raw_end = bobl::flyweight::lite::utility::details::end_raw(array);
	auto doc = flyweight::details::EmbeddedDocument::decode_as<bobl::bson::Array>(raw_begin, raw_end);
	auto value = doc.value();
	auto begin = value.begin();
	auto end = value.end();
	return boost::make_iterator_range(details::ArrayIterator<Array, T, Options>{begin, end}, details::ArrayIterator<Array, T, Options>{end, end});
}

template<typename T, typename Array>
auto make_iterator_range(Array const& array)
	-> typename std::enable_if<bobl::flyweight::lite::utility::IsArray<Array>::value, boost::iterator_range<details::ArrayIterator<Array, T, bobl::options::None>>>::type
{
	return make_iterator_range<T, bobl::options::None, Array>(array);
}

}/*namespace bson*/ } /*namespace bobl*/


