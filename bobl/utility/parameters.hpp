// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/options.hpp"
#include "bobl/options.hpp"
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/mpl/pop_back.hpp>
#include <boost/mpl/back.hpp>
#include <utility>
#include <tuple>
#include <type_traits>


namespace bobl{ namespace utility {

template<typename T, typename ...Args>
class CastParameters
{
	static_assert(!bobl::utility::IsOptions<T>::value, "at least one parameter required beside options");
	using Pack = std::tuple<T, Args...>;
	using Last = typename boost::mpl::back<Pack>::type;
	using Parameters = typename std::conditional< bobl::utility::IsOptions<Last>::value, typename boost::mpl::pop_back<Pack>::type, Pack>::type;
public:
	using Result = typename std::conditional<boost::fusion::result_of::size<Parameters>::value == 1,
																					T,
																					Parameters>::type;
	using Options = typename std::conditional< bobl::utility::IsOptions<Last>::value, Last, bobl::options::None>::type;
};

template<typename NsTag, typename T, typename ...Options>
using IsHeterogeneousArraySequence = bobl::utility::options::Contains<typename bobl::EffectiveOptions<NsTag, T, Options...>::type, bobl::options::HeterogeneousArray<T>>;

template<typename NsTag, typename T, typename ...Options>
using IsObjectSequence = boost::mpl::not_<IsHeterogeneousArraySequence<NsTag, T, Options...>>;

template<typename NsTag, typename ...Args>
struct DecodeParameters
{
	using Options = typename CastParameters<Args...>::Options;
	using Result = typename CastParameters<Args...>::Result;
	using Parameters = typename std::conditional< boost::mpl::and_<boost::fusion::traits::is_sequence<Result>, 
																		IsObjectSequence<NsTag, Result, Options>
													>::value,
													Result, std::tuple<Result>>::type;
	static auto result(std::tuple<Result>&& res) -> decltype(std::get<0>(std::move(res))) { return std::get<0>(std::move(res)); }
	template<typename T>
	static T&& result(T&& res) { return std::forward<T>(res); }
};

}/*namespace utility*/ } /*namespace bobl*/

