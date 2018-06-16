// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/details/encoder.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/options.hpp"
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <vector>
#include <type_traits>
#include <cstdint>
#include <cassert>


namespace bobl{ namespace bson { 


template<typename ...Options, typename T>
auto encode(T const& value)  
	-> typename std::enable_if<boost::mpl::and_<boost::fusion::traits::is_sequence<T>, 
								bobl::utility::NamedSequence<T, typename bobl::bson::EffectiveOptions<T, Options...>::type>>::value,
										std::vector<std::uint8_t>>::type
{
	std::vector<std::uint8_t> buffer;
	encoder::details::Handler<T, bobl::Options<Options...>>::encode(buffer, value);
	return buffer;
}

template<typename ...Options, typename T>
auto encode(T&& value)  
	-> typename std::enable_if<boost::mpl::and_<boost::mpl::not_<boost::fusion::traits::is_sequence<typename std::decay<T>::type>>,
								bobl::utility::NamedSequence<std::tuple<typename std::decay<T>::type>, typename bobl::bson::EffectiveOptions<std::tuple<typename std::decay<T>::type>, Options...>::type>>::value,
										std::vector<std::uint8_t>>::type
{
	return encode<Options...>(std::make_tuple(std::forward<T>(value)));
}


template<typename ...Options, typename ...Args>
auto encode(Args&& ...args)  
		-> typename std::enable_if<(sizeof ...(Args) > 1 ) && bobl::utility::NamedSequence<std::tuple<Args...>, typename bobl::bson::EffectiveOptions<std::tuple<Args...>, Options...>::type>::value,
																std::vector<std::uint8_t>>::type
{
	return encode<Options...>(std::make_tuple(std::forward<Args>(args)...));
}

template<typename ...Options, typename Iterator, typename T>
auto encode(Iterator out, T const& value)
	-> typename std::enable_if<
					boost::mpl::and_<std::is_same<typename std::iterator_traits<Iterator>::iterator_category, std::output_iterator_tag>,
									boost::fusion::traits::is_sequence<T>, 
									bobl::utility::NamedSequence<T, typename bobl::bson::EffectiveOptions<T, Options...>::type>>::value, Iterator>::type
{
	return encoder::details::Handler<T, bobl::Options<Options...>>::encode(out, value);
}

template<typename ...Options, typename Iterator, typename ...Args>
auto encode(Iterator out, Args&& ...args)
		-> typename std::enable_if<
				boost::mpl::and_<std::is_same<typename std::iterator_traits<Iterator>::iterator_category, std::output_iterator_tag>,
				bobl::utility::NamedSequence<std::tuple<Args...>,typename bobl::bson::EffectiveOptions<std::tuple<Args...>, Options...>::type>>::value, Iterator>::type
{
	return encode<Options...>(out, std::make_tuple(std::forward<Args>(args)...));
}



} /*namespace bson*/ }/*namespace bobl*/

