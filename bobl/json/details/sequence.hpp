// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/json/details/parser.hpp"
#include "bobl/json/adapter.hpp"
#include "bobl/json/options.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/flyweight.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/decoders.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/bobl.hpp"
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <string>
#include <cassert>
//#include <boost/range/iterator_range.hpp>
//#include <string>

namespace bobl{ namespace json { namespace decoder { namespace details {
	
template<typename T, typename Options> class NameValue;

template<typename T, typename Options, typename Enabled = boost::mpl::true_>
class Handler {};

template<typename T, typename Options>
class Decoder
{
	using Type = typename std::conditional<bobl::utility::Adaptable<T, bobl::json::Adapter<T>>::value, bobl::json::Adapter<T>, T>::type;
public:
	using type = Handler<Type, typename bobl::json::EffectiveOptions<T, Options>::type>;
};

template<typename T, typename Options, typename Enabled = boost::mpl::true_>
class SequenceHandler {};

template<typename Iterator, typename ...Options>
class ObjectDecoder
{
	template<typename T>
	using NameValue = bobl::json::decoder::details::NameValue<T, bobl::Options<Options...>>;
public:
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = ObjectDecoder<OtherIterator, OtherOptions...>;

	template<typename T, std::size_t N, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<N==0, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{})
	{
		return NameValue<T>::decode(begin, end, ename).value();
	}

	template<typename T, std::size_t N, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<N != 0, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{})
	{
		parser::Delimiter::parse(begin, end);
		return decode<T, 0, ExpectedName>(begin, end, ename);
	}
};

template<typename T, typename ...Options>
class SequenceHandler<T, bobl::Options<Options...>, typename boost::mpl::or_<
																	boost::mpl::not_<bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::StructAsDictionary>>,
																	boost::mpl::not_<bobl::utility::DictionaryDecoderCompatible<T, 
																						typename bobl::json::EffectiveOptions<T, Options...>::type>>>::type>
{
	static_assert(boost::fusion::traits::is_sequence<T>::value, "SequenceHandler requires boost mpl sequence");
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		auto res = bobl::utility::Decoder<T, ObjectDecoder<Iterator>, Options...>{}(begin, end);

		if /*constexpr*/ (bobl::utility::options::Contains<bobl::Options<Options...>,bobl::options::ExacMatch>::value)
		{
			if (!boost::spirit::qi::phrase_parse(begin, end, boost::spirit::ascii::char_('}'), boost::spirit::ascii::space))
#pragma message("TODO:parse next object name and extend error message below")
#pragma message("TODO:may be more correct message would be:JSON object closing '}' is missing")
				throw bobl::InvalidObject{ "json object contains more child objects than expected" };
		}
		else
		{
			if (!boost::spirit::qi::phrase_parse(begin, end, -(',' >> bobl::json::parser::JsonParsers<Iterator>::name_value() % ',') >> '}', boost::spirit::ascii::space))
				throw bobl::InvalidObject{ "JSON object has invalid sintaxis" };
		}

		return res;
	}
};

template<typename Iterator, typename ...Options>
class DictionaryObjectDecoder
{
public:
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = DictionaryObjectDecoder<OtherIterator, OtherOptions...>;

	using Name = std::string;
	using Skipper = bobl::flyweight::lite::Any<Iterator>;

	static Name const& name(Name const& name) { return name; }
	static Name decode_name(Iterator& begin, Iterator end) { return bobl::json::parser::Name::parse(begin, end); }

	template<typename T>
	static auto decode(Name&& name, Iterator& begin, Iterator end) -> typename std::enable_if<bobl::flyweight::utility::IsNameValue<T>::value, T>::type
	{
		using Decoder = typename Decoder<T, bobl::Options<Options...>>::type;
		return { std::move(name), Decoder::decode(begin, end) };
	}

	template<typename T>
	static auto decode(Name&& /*name*/, Iterator& begin, Iterator end) -> typename std::enable_if<!bobl::flyweight::utility::IsNameValue<T>::value, T>::type
	{
		using Decoder = typename Decoder<T, bobl::Options<Options...>>::type;
		return Decoder::decode(begin, end);
	}
};

template<typename T, typename ...Options>
class SequenceHandler<T, bobl::Options<Options...>, typename boost::mpl::and_<bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::StructAsDictionary>,
																				bobl::utility::DictionaryDecoderCompatible<T, 
																					typename bobl::json::EffectiveOptions<T, Options...>::type>>::type>
{
	static_assert(boost::fusion::traits::is_sequence<T>::value, "SequenceHandler requires boost mpl sequence");
	struct EndOfObject
	{
		template<typename Iterator>
		bool operator()(Iterator& begin, Iterator end) 
		{
			struct Delimiters : boost::spirit::qi::symbols<char, bool >
			{
				Delimiters()
				{
					add
						(",", false )
						("}", true)
						;
				}
			} static const parser;

			bool res = i++ != 0;
			if (res && !boost::spirit::qi::phrase_parse(begin, end, parser, boost::spirit::ascii::space, res))
				throw bobl::InvalidObject{ "delimiter is missing" };
			return res;
		}
	private:
		int i = 0;
	}; 
public:
	template<typename Iterator>
	static T decode(Iterator& begin, Iterator end)
	{
		return bobl::utility::DictionaryDecoder<T, DictionaryObjectDecoder<Iterator>, Options...>{}(begin, end, EndOfObject{});
	}
};


} /*namespace details*/} /*namespace decoder*/ }/*namespace json*/ } /*namespace bobl*/
