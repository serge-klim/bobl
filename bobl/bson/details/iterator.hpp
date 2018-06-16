// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/bson/flyweight.hpp"
#include "bobl/utility/iterator.hpp"
#include "bobl/names.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace bson { namespace details{

template <typename T> struct Traits;

struct NameValueDecoder
{
	template<typename T, typename Options, typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		return bobl::bson::flyweight::NameValue<T, Options>::decode(begin, end, bobl::utility::ObjectNameIrrelevant{}).value();
	}
};

template<typename T, typename Options, typename Iterator>
boost::iterator_range<bobl::utility::Iterator<T, Options, NameValueDecoder, Iterator>> make_iterator_range(Iterator begin, Iterator end)
{
	return boost::make_iterator_range(bobl::utility::Iterator<T, Options, NameValueDecoder, Iterator>{begin, end},
														bobl::utility::Iterator<T, Options, NameValueDecoder, Iterator>{end, end});
}


template<typename Object, typename T, typename Options>
using ObjectIterator = typename Traits<Object>::template Iterator<T, Options>;

template<typename Array, typename T, typename Options>
using ArrayIterator = typename Traits<Array>::template Iterator<T, Options>;


template <typename T> 
struct Traits
{
};

template<>
struct Traits<flyweight::Document>
{
	template<typename U, typename Options = bobl::options::None>
	using Iterator = bobl::utility::Iterator<U, Options, details::NameValueDecoder, bobl::bson::flyweight::Iterator>;
};

template<typename RawIterator>
struct Traits<bobl::flyweight::lite::Array<RawIterator>>
{
	template<typename U, typename Options = bobl::options::None>
	using Iterator = bobl::utility::Iterator<U, Options, details::NameValueDecoder, RawIterator>;
};


template<typename RawIterator>
struct Traits<bobl::flyweight::lite::Object<RawIterator>>
{
	template<typename U, typename Options = bobl::options::None>
	using Iterator = bobl::utility::Iterator<U, Options, details::NameValueDecoder, RawIterator>;
};

} /*namespace details*/ }/*namespace bson*/ } /*namespace bobl*/


