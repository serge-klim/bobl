// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/cbor/details/decoder.hpp"
#include "bobl/utility/names.hpp"


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


} /*namespace details*/ }/*namespace cbor*/ } /*namespace bobl*/


