// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/json/details/decoder.hpp"
#include "bobl/json/details/parser.hpp"
#include "bobl/json/details/options.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/bobl.hpp"
#include <type_traits>

namespace bobl{ namespace json { 

using Type = bobl::Type;
using bobl::to_string;

template<typename Iterator, bobl::flyweight::utility::AnyTag Tag>
Type type(bobl::flyweight::lite::utility::AnyType<Iterator, Tag> const& any)
{
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(any);
	auto end = bobl::flyweight::lite::utility::details::end_raw(any);
	Type res = Type::Null;
	if (begin != end)
	{
		if (!boost::spirit::qi::phrase_parse(begin, end, bobl::json::parser::JsonParsers<Iterator>::value_type(), boost::spirit::ascii::space, res))
			throw bobl::InvalidObject{ "can't parse JSON value" };
	}
	return res;
}

template<typename ...Args, typename Iterator>
auto decode(Iterator& begin, Iterator end) -> typename bobl::utility::DecodeParameters<bobl::json::NsTag, Args...>::Result
{
	using Parameters = typename bobl::utility::DecodeParameters<bobl::json::NsTag, Args...>::Parameters;
	using Options = typename bobl::utility::DecodeParameters<bobl::json::NsTag, Args...>::Options;
	using Decoder = typename decoder::details::Decoder<bobl::json::NsTag, Parameters, Options>::type;
	return bobl::utility::DecodeParameters<bobl::json::NsTag, Args...>::result(Decoder::decode(begin, end));
}

}/*namespace json*/ } /*namespace bobl*/