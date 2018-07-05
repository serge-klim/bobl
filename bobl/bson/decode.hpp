// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/bson/details/options.hpp"
#include "bobl/bson/details/header.hpp"
#include "bobl/bson/flyweight.hpp"
#include "bobl/bson/bson.hpp"
#include "bobl/utility/any.hpp"
#include "bobl/bobl.hpp"

namespace bobl{ namespace bson { 

template<typename Iterator>
Type native_type(bobl::flyweight::lite::Any<Iterator> const& any)
{
	auto begin = bobl::bson::flyweight::Iterator(bobl::flyweight::lite::utility::details::begin_raw(any));
	auto end = bobl::bson::flyweight::Iterator(bobl::flyweight::lite::utility::details::end_raw(any));
	return flyweight::details::ObjectHeader{begin,end}.type();
}

template<typename Iterator>
bobl::Type type(bobl::flyweight::lite::Any<Iterator> const& any)
{
	auto res = bobl::Type::Null;
	switch (auto type = native_type(any))
	{
		case bobl::bson::Null:
			res = bobl::Type::Null;
			break;
		case bobl::bson::Double:
			res = bobl::Type::Double;
			break;
		case bobl::bson::Utf8String:
		case bobl::bson::JavaScript:
		case bobl::bson::Symbol:
			res = bobl::Type::String;
			break;
		case bobl::bson::Binary:
			res = bobl::Type::Binary;
			break; 
		case bobl::bson::Bool:
			res = bobl::Type::Bool;
			break; 
		case bobl::bson::UTCDateTime:
			res = bobl::Type::Timepoint;
			break; 
		case bobl::bson::Int32:
			res = bobl::Type::Int32;
			break; 
		case bobl::bson::Int64:
			res = bobl::Type::Int64;
			break;
		case bobl::bson::EmbeddedDocument:
			res = bobl::Type::Dictionary;
			break;
		case bobl::bson::Array:
			res = bobl::Type::Array;
			break;
		default:
			throw bobl::TypeNotSupported(str(boost::format("unsupported type %1% (%2$#x)") % to_string(type) % int(type)));
			break;
	}
	return res;
}

template<typename ...Args, typename Iterator>
auto decode(Iterator& begin, Iterator end) -> typename bobl::utility::DecodeParameters<bobl::bson::NsTag, Args...>::Result
{
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	return cast<Args...>(doc);
}

}/*namespace bson*/ } /*namespace bobl*/


