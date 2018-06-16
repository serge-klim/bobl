// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/json/details/sequence.hpp"
#include "bobl/json/details/parser.hpp"
#include "bobl/json/adapter.hpp"
#include "bobl/utility/utils.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/decoders.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/bobl.hpp"
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/format.hpp>
#include<boost/range/iterator.hpp>
#include <vector>
#include <string>
#include <type_traits>
#include <cstdint>
#include <cassert>


namespace bobl{ namespace json { namespace decoder { namespace details {

template<typename T, typename Options>
class Handler<bobl::flyweight::lite::Any<T>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static bobl::flyweight::lite::Any<T> decode(Iterator& begin, Iterator end)
	{
		boost::iterator_range<T> value;
		if (!boost::spirit::qi::phrase_parse(begin, end, bobl::json::parser::JsonParsers<T>::value(), boost::spirit::ascii::space, value))
			throw bobl::InvalidObject{ "can't parse JSON value" };
		return {std::move(value)};
	}
};

template<typename T, typename Options>
class Handler<bobl::flyweight::lite::Array<T>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static bobl::flyweight::lite::Array<T> decode(Iterator& begin, Iterator end)
	{
		boost::iterator_range<T> value;
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::qi::raw[bobl::json::parser::JsonParsers<T>::array()], boost::spirit::ascii::space, value))
			throw bobl::InvalidObject{ "can't parse JSON array" };
		return { std::move(value) };
	}
};

template<typename T, typename Options>
class Handler<bobl::flyweight::lite::Object<T>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static bobl::flyweight::lite::Object<T> decode(Iterator& begin, Iterator end)
	{
		boost::iterator_range<T> value;
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::qi::raw[bobl::json::parser::JsonParsers<T>::object()], boost::spirit::ascii::space, value))
			throw bobl::InvalidObject{ "can't parse JSON object" };
		return { std::move(value) };
	}
};

template<typename Options>
class Handler<bool, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static bool decode(Iterator& begin, Iterator end)
	{
		bool res;
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::qi::bool_, boost::spirit::ascii::space, res))
			throw bobl::InvalidObject{ "can't parse JSON bool" };
		return res;
	}
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_integral<T>::value>  >
{
    template<unsigned Radix>
    using BaseParser = typename std::conditional<std::is_signed<T>::value, boost::spirit::qi::int_parser<T, Radix>, boost::spirit::qi::uint_parser<T, Radix>>::type;

	template<typename Iterator>
    struct StrictParser : boost::spirit::qi::grammar<Iterator, T(), boost::spirit::qi::ascii::space_type>
    {
        StrictParser() : StrictParser::base_type(parser)  {  parser = BaseParser<10>{}; }
        boost::spirit::qi::rule<Iterator, T(), boost::spirit::qi::ascii::space_type> parser;
    };

	template<typename Iterator>
    struct XParser : boost::spirit::qi::grammar<Iterator, T(), boost::spirit::qi::ascii::space_type>
    {
        XParser() : XParser::base_type(parser)
        {
            parser
                = (boost::spirit::qi::lit('"') >>
                (
                    ((boost::spirit::qi::no_case["0x"] | boost::spirit::qi::lit("#")) >> boost::spirit::qi::uint_parser<T, 16>{})
                    | (boost::spirit::qi::uint_parser<T, 2>{} >> (boost::spirit::qi::no_case["b"]))
                    | BaseParser<10>{}) >> boost::spirit::qi::lit('"'))
                | BaseParser<10>{};
        }
        boost::spirit::qi::rule<Iterator, T(), boost::spirit::qi::ascii::space_type> parser;
    };
public:
 	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		using Parser = typename std::conditional<bobl::utility::options::Contains<typename bobl::json::EffectiveOptions<T, Options>::type, bobl::options::RelaxedIntegers>::value, XParser<Iterator>, StrictParser<Iterator>>::type;
		auto res = T{};
		if (!boost::spirit::qi::phrase_parse(begin, end, Parser{}, boost::spirit::ascii::space, res))
			throw bobl::InvalidObject{ "can't parse JSON double" };
		return res;
	}
};

template<typename T, typename Options>
class Handler<T, Options, boost::mpl::bool_<std::is_floating_point<T>::value>  >
{
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		double res;
		if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::qi::double_, boost::spirit::ascii::space, res))
			throw bobl::InvalidObject{ "can't parse JSON floating point" };
		return T(res);
	}
};

template<typename T, typename Options>
class Handler<std::basic_string<T>, Options, boost::mpl::true_> 
{
public:
	template<typename Iterator>
	static std::basic_string<T> decode(Iterator& begin, Iterator end)
	{
		auto res = std::basic_string<T>{};
		if (!boost::spirit::qi::phrase_parse(begin, end, bobl::json::parser::String<Iterator, T>{}, boost::spirit::ascii::space, res))
			throw bobl::InvalidObject{ "can't parse JSON string" };
		return res;
	}
};

template<typename T, typename Options>
class Handler<std::vector<T>, Options, boost::mpl::true_>
{
public:
	template<typename Iterator>
	static std::vector<T> decode(Iterator& begin, Iterator end)
	{
		std::vector<boost::iterator_range<Iterator>> jarray;
		if (!boost::spirit::qi::phrase_parse(begin, end, bobl::json::parser::JsonParsers<Iterator>::array(), boost::spirit::ascii::space, jarray))
			throw bobl::InvalidObject{ "can't parse JSON array" };

		std::vector<T> res;
		std::transform(std::begin(jarray), std::end(jarray), std::back_inserter(res), [](boost::iterator_range<Iterator> const& range)
		{
			using Decoder = typename Decoder<T, Options>::type;
			auto begin = range.begin();
			return Decoder::decode(begin, range.end());
		});
		return res;
	}
};

template<typename ...Params, typename Options>
class Handler<diversion::variant<Params...>, Options, boost::mpl::true_>
{
	static_assert(sizeof...(Params) != 0, "there is no much sense to parse empty variant");
	using ValueType = diversion::variant<Params...>;
public:
	template<typename Iterator>
	static ValueType decode(Iterator& begin, Iterator end)
	{
		if (begin == end)
		{
			auto param_list = bobl::utility::type_name<Params...>();
			throw bobl::InputToShort{ str(boost::format("not enought data to decode JSON variant <%1%>") % param_list) };
		}
		return try_decode<Iterator, Params...>(begin, end);
	}
private:
	template<typename Iterator, typename T, typename ...Args >
	static ValueType try_decode(Iterator& begin, Iterator end)
	{
		auto i = begin;
		try
		{
			using Decoder = typename Decoder<T, Options>::type;
			return { Decoder::decode(begin, end) };
		}
		catch (bobl::IncorrectObjectType&)
		{
		}
		begin = i;
		return try_decode<Iterator, Args...>(begin, end);
	}

	template<typename Iterator>
	static ValueType try_decode(Iterator& begin, Iterator end)
	{
		assert(begin != end);
#pragma message("TODO:perhaps make better error message here: type related !!!")
		//auto type = bobl::cbor::utility::decode::major_type(*begin);
		auto param_list = bobl::utility::type_name<Params...>();
		//throw bobl::IncorrectObjectType{ str(boost::format("JSON value has unexpected type : %1% (%2$#x) insted of expected variant <%3%>") % to_string(type) % int(type) % param_list) };
		throw bobl::InputToShort{ str(boost::format("can't decode JSON variant <%1%>") % param_list) };
	}
};


template<typename T, typename Options>
class NameValue 
{
	NameValue(std::string name, T&& value) : name_{ std::move(name) }, value_(std::move(value)) {}
public:
	std::string const& name() const { return name_; }
	T const& value() const & { return value_; }
#if !defined(BOOST_GCC_VERSION) || BOOST_GCC_VERSION > 50000 
	//https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60943
	T value() const && { return std::move(value_); }
#endif

	template<typename Iterator, typename NameType>
	static NameValue decode(Iterator& begin, Iterator end, NameType const& ename)
	{
		auto name = bobl::json::parser::Name::parse(begin, end);
		if (!ename.compare(name))
			throw bobl::IncorrectObjectName{ str(boost::format("unexpected JSON object name : \"%1%\" (expected \"%2%\").") % name % ename.name()) };

		using Decoder = typename Decoder<T, Options>::type;
		return { std::move(name), Decoder::decode(begin, end) };
	}
private:
	std::string name_;
	T value_;
};

//template<typename T, typename Options>
//class NameValue<diversion::optional<T>, Options>
//{
//};

template<typename T, typename Options>
class NameValue<bobl::flyweight::NameValue<T>, Options> : public NameValue<T, Options>
{
	using Base = NameValue<T, Options>;
	NameValue(Base&& base) : Base{ std::move(base) } {}
public:
	bobl::flyweight::NameValue<T> value() const& { return { std::string(Base::name()), Base::value() }; }
	bobl::flyweight::NameValue<T> value() && { return { std::string(Base::name()), Base::value() }; }
	template<typename Iterator, typename NameType>
	static NameValue decode(Iterator& begin, Iterator end, NameType const& ename) { return { Base::decode(begin, end, ename) }; }
};

template<typename T, typename Options>
class Handler<T, Options, typename boost::mpl::and_<typename boost::fusion::traits::is_sequence<T>::type,
								boost::mpl::not_<bobl::utility::options::Contains<typename bobl::json::EffectiveOptions<T, Options>::type, bobl::options::NonUniformArray>>>::type>
{
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		parser::ObjectOpen::parse(begin, end);
		return SequenceHandler<T, Options>::decode(begin, end);
	}
};

template<typename T, typename Options>
class Handler<bobl::json::Adapter<T>, Options, boost::mpl::true_>
{
	using Adaptee = Handler<typename bobl::json::Adapter<T>::type, typename json::EffectiveOptions<T, Options>::type>;
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		auto value = Adaptee::decode(begin, end);
		return bobl::json::Adapter<T>{}(std::move(value));
	}
};

} /*namespace details*/} /*namespace decoder*/ }/*namespace json*/ } /*namespace bobl*/


