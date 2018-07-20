// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/cbor/details/decoder.hpp"
#include "bobl/utility/iterator.hpp"


namespace bobl{ namespace cbor { 

template <typename T> struct Traits;

namespace details{

struct ValueDecoder
{
	template<typename T, typename Options, typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		return bobl::cbor::decoder::details::Handler<T, Options>::template decode<Iterator>(begin, end);
	}
};

struct NameValueDecoder
{
	template<typename T, typename Options, typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		return decoder::details::NameValue<T, Options>::template decode<Iterator>(begin, end, bobl::utility::ObjectNameIrrelevant{}).value();
	}
};


template<typename Object, typename T, typename Options>
using ObjectIterator = typename Traits<Object>::template Iterator<T, Options>;

template<typename Array, typename T, typename Options>
using ArrayIterator = typename Traits<Array>::template Iterator<T, Options>;


} /*namespace details*/ 

template <typename T>
struct Traits {};

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


}/*namespace cbor*/ } /*namespace bobl*/


