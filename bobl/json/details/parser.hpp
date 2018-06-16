// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/bobl.hpp"

#include <boost/fusion/include/iterator_range.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/fusion/include/iterator_range.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <iterator>
#include <utility>
#include <vector>
#include <string>
#include <type_traits>
 
namespace bobl{ namespace json{ namespace parser{ 

template <typename Iterator, typename Char = char>
struct String : boost::spirit::qi::grammar<Iterator, std::basic_string<Char>(), boost::spirit::qi::ascii::space_type>
{
	String() : String::base_type(string)
    {
 	    using boost::spirit::qi::lexeme;
 	    using boost::spirit::ascii::char_;
 
 	    escapes = lexeme['\\' > char_('"')];
 	    string = lexeme['"' >> *(escapes | (char_ - '"')) >> '"'];
    }
 
    boost::spirit::qi::rule<Iterator> escapes;
    boost::spirit::qi::rule<Iterator, std::basic_string<Char>(), boost::spirit::qi::ascii::space_type> string;
};
 
template <typename Iterator>
class JsonParsers 
{
    JsonParsers() //: JsonObjectParser::base_type(object)
    {
        using boost::spirit::qi::long_;
        using boost::spirit::qi::double_;
        using boost::spirit::ascii::char_;
        using boost::spirit::ascii::digit;
        using boost::spirit::qi::lexeme;
        using boost::spirit::qi::raw;
		using boost::spirit::attr;
		using boost::spirit::omit;

        escapes_ = lexeme['\\' > char_('"')];

        string_ = raw['"' >> *(escapes_ | (char_ - '"')) >> '"'];

        value_ = raw[string_ | "true" | "false" | "null" | double_ | long_ | jarray_ | object_];

		value_type_ = (omit[string_] >> attr(bobl::Type::String)) 
					| ("true" >> attr(bobl::Type::Bool)) | ("false" >> attr(bobl::Type::Bool)) 
					| ("false" >> attr(bobl::Type::Null))
					| (omit[double_] >> attr(bobl::Type::Double)) 
					| (omit[long_] >> attr(bobl::Type::Integer))
					| (omit[jarray_] >> attr(bobl::Type::Array))
					| (omit[object_] >> attr(bobl::Type::Dictionary));	

        jarray_ = '[' >> -(value_ % ',') >> ']';

		name_value_ = (string_ >> ':' >> value_);

		object_ = '{' >> -(name_value_ % ',') >> '}';
    }
public:
    static JsonParsers const& Instance()
    { 
        static JsonParsers const parser;
        return parser;
    }
	static boost::spirit::qi::rule<Iterator, bobl::Type(), boost::spirit::qi::ascii::space_type> const& value_type() { return Instance().value_type_; }

    static boost::spirit::qi::rule<Iterator, boost::iterator_range<Iterator>()> const& string() { return Instance().string_; }
    static boost::spirit::qi::rule<Iterator, boost::iterator_range<Iterator>(), boost::spirit::qi::ascii::space_type> const& value() { return Instance().value_; }
    static boost::spirit::qi::rule<Iterator, std::vector<boost::iterator_range<Iterator>>(), boost::spirit::qi::ascii::space_type> const& array() { return Instance().jarray_; }
	static boost::spirit::qi::rule<Iterator, std::pair<boost::iterator_range<Iterator>, boost::iterator_range<Iterator>>(), boost::spirit::qi::ascii::space_type> const& name_value() { return Instance().name_value_; }
    static boost::spirit::qi::rule<Iterator, std::vector<std::pair<boost::iterator_range<Iterator>, boost::iterator_range<Iterator>>>(), boost::spirit::qi::ascii::space_type> const& object() { return Instance().object_; }
private:
    boost::spirit::qi::rule<Iterator> escapes_;
    boost::spirit::qi::rule<Iterator, boost::iterator_range<Iterator>()> string_;
    boost::spirit::qi::rule<Iterator, boost::iterator_range<Iterator>(), boost::spirit::qi::ascii::space_type> value_;
	boost::spirit::qi::rule<Iterator, std::pair<boost::iterator_range<Iterator>, boost::iterator_range<Iterator>>(), boost::spirit::qi::ascii::space_type> name_value_;
    boost::spirit::qi::rule<Iterator, std::vector<boost::iterator_range<Iterator>>(), boost::spirit::qi::ascii::space_type> jarray_;
    boost::spirit::qi::rule<Iterator, std::vector<std::pair<boost::iterator_range<Iterator>, boost::iterator_range<Iterator>>>(), boost::spirit::qi::ascii::space_type> object_;

	boost::spirit::qi::rule<Iterator, bobl::Type(), boost::spirit::qi::ascii::space_type> value_type_;
};

struct Name
{
	template<typename Iterator>
	static std::string parse(Iterator& begin, Iterator end)
	{
		auto name = std::string{};
		if (!boost::spirit::qi::phrase_parse(begin, end, bobl::json::parser::String<Iterator>{} >> ':', boost::spirit::ascii::space, name))
			throw bobl::InvalidObject{ "can't parse JSON object name" };
		return name;
	}
};


//template<typename Iterator>
//struct Type
//{
//	Type() : Type::base_type(start)
//	{
//		static const boost::spirit::attr(json::Type::NameValue) jobject;
//		static const boost::spirit::attr(json::Type::Array) jarray;
//		static const boost::spirit::attr(json::Type::String) jstring;
//		static const boost::spirit::attr(json::Type::Bool) jbool;
//		static const boost::spirit::attr(json::Type::Jnull) jnull;
//		keyword.add
//			("{", &object)
//			("[", &jarray)
//			("\"", &jstring)
//			("t", &jbool)
//			("f", &jbool)
//			("n", &jnull)
//			;
//
//		start_ = double_ | long_ | (*(keyword[boost::spirit::qi::_a = _1] >> lazy(*boost::spirit::qi::_a)))
//	}
//	boost::spirit::qi::symbols<char, boost::spirit::qi::rule<Iterator, boost::spirit::ascii::space_type>*> keyword;
//};

struct Delimiter
{
	template<typename Iterator>
	static void parse(Iterator& begin, Iterator end)
	{
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::ascii::char_(','), boost::spirit::ascii::space))
			throw bobl::InvalidObject{ "JSON delimiter is missing" };
	}
};

struct ObjectOpen
{
	template<typename Iterator>
	static void parse(Iterator& begin, Iterator end)
	{
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::ascii::char_('{'), boost::spirit::ascii::space))
			throw bobl::InvalidObject{ "JSON object opening '{' is missing" };	}
};

struct ArrayOpen
{
	template<typename Iterator>
	static void parse(Iterator& begin, Iterator end)
	{
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::ascii::char_('['), boost::spirit::ascii::space))
			throw bobl::InvalidObject{ "JSON array opening '[' is missing" };	}
};


} /*namespace parser*/ } /*namespace json*/ } /*namespace bobl*/
