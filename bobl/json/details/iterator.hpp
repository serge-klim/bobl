// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/json/details/decoder.hpp"
#include "bobl/utility/iterator.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/names.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"
#include <boost/spirit/home/qi/skip_over.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/format.hpp>
#include <iterator>

namespace bobl{ namespace json { 

template <typename T> struct Traits;

namespace details{

struct NameValueDecoder
{
	template<typename T, typename Options, typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		using ValueType = typename std::iterator_traits<Iterator>::value_type;
		using NameValue = bobl::json::decoder::details::NameValue<T, Options>;
		auto res = NameValue::decode(begin, end, bobl::utility::ObjectNameIrrelevant{}).value();
		boost::spirit::qi::skip_over(begin, end, boost::spirit::compile<boost::spirit::qi::domain>(boost::spirit::ascii::space));
		if(begin == end)
			throw bobl::InvalidObject{ "JSON object closing '}' is missing" };
		switch (*begin)
		{
			case ValueType(','):
				++begin;
				break;
			case ValueType('}'):
				boost::spirit::qi::skip_over(++begin, end, boost::spirit::compile<boost::spirit::qi::domain>(boost::spirit::ascii::space));
				if (begin != end)
					throw bobl::InvalidObject{ str(boost::format("JSON object unexpected symbol found \"%1%\"") % char(*begin)) };
				break;
			default:
				throw bobl::InvalidObject{ "JSON delimiter is missing" };
		}
		return res;
	}
};

struct ValueDecoder
{
	template<typename T, typename Options, typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		using ValueType = typename std::iterator_traits<Iterator>::value_type;
		using Decoder = typename decoder::details::Decoder<T, Options>::type;
		auto res = Decoder::decode(begin, end);
		boost::spirit::qi::skip_over(begin, end, boost::spirit::compile<boost::spirit::qi::domain>(boost::spirit::ascii::space));
		if (begin == end)
			throw bobl::InvalidObject{ "JSON array closing ']' is missing" };
		switch (*begin)
		{
			case ValueType(','):
				++begin;
				break;
			case ValueType(']'):
				boost::spirit::qi::skip_over(++begin, end, boost::spirit::compile<boost::spirit::qi::domain>(boost::spirit::ascii::space));
				if (begin != end)
					throw bobl::InvalidObject{ str(boost::format("JSON array unexpected symbol found \"%1%\"") % char(*begin)) };
				break;
			default:
				throw bobl::InvalidObject{ "JSON delimiter is missing" };
		}
		return res;
	}
};


template<typename Object, typename T, typename Options>
using ObjectIterator = typename Traits<Object>::template Iterator<T, Options>;

template<typename Array, typename T, typename Options>
using ArrayIterator = typename Traits<Array>::template Iterator<T, Options>;


} /*namespace details*/ }/*namespace json*/ } /*namespace bobl*/


