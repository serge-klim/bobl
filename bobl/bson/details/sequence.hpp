// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/bson/details/header.hpp"
#include "bobl/bson/adapter.hpp"
#include "bobl/bson/options.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/utility/decoders.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/bobl.hpp"
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/format.hpp>
#include <cassert>

namespace bobl{ namespace bson { namespace flyweight { 
	
template<typename T, typename Options> class NameValue;
	
namespace details{ 

template<typename T> struct IsNameValue : std::false_type {};
template<typename T, typename Options> struct IsNameValue<bobl::bson::flyweight::NameValue<T, Options>> : std::true_type {};

template<typename T, bool HeterogeneousArray, typename Options, typename Enabled = boost::mpl::true_>
class SequenceHandler {};

template<typename Iterator, bool HeterogeneousArray, typename ...Options>
class ObjectDecoder
{
	template<typename T>
	using NameValue = bobl::bson::flyweight::NameValue<T, typename bobl::bson::EffectiveOptions<T, Options...>::type>;
public:
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = ObjectDecoder<OtherIterator, HeterogeneousArray, OtherOptions...>;

	template<typename T, std::size_t, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<!HeterogeneousArray && IsNameValue<T>::value, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{}) const
	{
		return T::decode(begin, end, ename);
	}

	template<typename T, std::size_t, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<!HeterogeneousArray && !IsNameValue<T>::value, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& ename = bobl::utility::ObjectNameIrrelevant{}) const
	{
		return NameValue<T>::decode(begin, end, ename).value();
	}

	template<typename T, std::size_t, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<HeterogeneousArray && IsNameValue<T>::value, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& /*ename*/ = bobl::utility::ObjectNameIrrelevant{}) const
	{
		return T::decode(begin, end, bobl::utility::ObjectNameIrrelevant{});
	}

	template<typename T, std::size_t, typename ExpectedName = bobl::utility::ObjectNameIrrelevant>
	typename std::enable_if<HeterogeneousArray && !IsNameValue<T>::value, T>::type decode(Iterator& begin, Iterator end, ExpectedName const& /*ename*/ = bobl::utility::ObjectNameIrrelevant{}) const
	{
		return NameValue<T>::decode(begin, end, bobl::utility::ObjectNameIrrelevant{}).value();
	}
};

template<typename T, bool HeterogeneousArray, typename ...Options>
class SequenceHandler<T, HeterogeneousArray, bobl::Options<Options...>, typename boost::mpl::or_<
																	boost::mpl::not_<bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::StructAsDictionary>>,
																	boost::mpl::not_<bobl::utility::DictionaryDecoderCompatible<T,
																		typename bobl::bson::EffectiveOptions<T, Options...>::type>>>::type>
{
	static_assert(boost::fusion::traits::is_sequence<T>::value, "SequenceHandler requires boost mpl sequence");
	template <typename Iterator>
	using ObjectDecoder = ObjectDecoder<Iterator, HeterogeneousArray, Options...>;
public:
	template<typename Iterator>
	static T decode(Iterator begin, Iterator end)
	{
		auto res = bobl::utility::Decoder<T, ObjectDecoder<Iterator>, Options...>{}(begin, end);
		if /*constexpr*/ (bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::ExacMatch>::value)
		{
			if (begin != end)
			{
				auto header = details::ObjectHeader{ begin, end };
				auto type = header.type();
				throw bobl::IncorrectObjectType{ str(boost::format("BSON document contains unexpected extra object %1% type : %2% (%3$#x)") % header.name() % to_string(type) % int(type)) };
			}
		}
		return res;
	}
};


template<typename Iterator, typename ...Options>
class DictionaryObjectDecoder 
{
	template<typename T>
	using NameValue = bobl::bson::flyweight::NameValue<T, typename bobl::bson::EffectiveOptions<T, Options...>::type>;
public:
	template<typename OtherIterator, typename ...OtherOptions>
	using rebase = DictionaryObjectDecoder<OtherIterator, OtherOptions...>;
	using Skipper = bobl::flyweight::lite::Any<Iterator>;
	using Name = ObjectHeader;

	static diversion::string_view name(ObjectHeader const& header) { return header.name(); }
	static ObjectHeader decode_name(Iterator& begin, Iterator end) { return ObjectHeader{ begin, end }; }

	template<typename T>
	static T decode(ObjectHeader&& header, Iterator& begin, Iterator end)
	{ 
		begin = header.position();
		return decode_<T>(std::move(header), begin, end);
	}
private:
	template<typename T>
	static typename std::enable_if<IsNameValue<T>::value, T>::type decode_(ObjectHeader&& header, Iterator& begin, Iterator end)
	{
		return T::decode(std::move(header), begin, end);
	}

	template<typename T>
	static typename std::enable_if<!IsNameValue<T>::value, T>::type decode_(ObjectHeader&& header, Iterator& begin, Iterator end)
	{
		return NameValue<T>::decode(std::move(header), begin, end).value();
	}

};

template<typename T, bool HeterogeneousArray, typename ...Options>
class SequenceHandler<T, HeterogeneousArray, bobl::Options<Options...>, typename boost::mpl::and_<bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::StructAsDictionary>,
																				bobl::utility::DictionaryDecoderCompatible<T, 
																				typename bobl::bson::EffectiveOptions<T, Options...>::type>>::type>
{
	static_assert(boost::fusion::traits::is_sequence<T>::value, "SequenceHandler requires boost mpl sequence");
public:
	template<typename Iterator>
	static T decode(Iterator begin, Iterator end)
	{
		return bobl::utility::DictionaryDecoder<T, DictionaryObjectDecoder<Iterator>, Options...>{}(begin, end);
	}
};



} /*namespace details*/ } /*namespace flyweight*/ } /*namespace bson*/ } /*namespace bobl*/

