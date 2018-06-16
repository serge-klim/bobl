// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/options.hpp"
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <type_traits>
#include <cstdint>

namespace bobl{ namespace utility {

template<typename ...Params>
struct IsOptions : std::false_type {};

template<typename ...Params>
struct IsOptions<Options<Params...>> : std::true_type {};

namespace options {

template<typename Options, typename T>
struct Contains : std::false_type {};

namespace details
{

template< typename Sequence, typename T>
struct ContainsImp
{
	using end = typename boost::mpl::end<Sequence>::type;
	using type = typename boost::mpl::not_<std::is_same<end,
													typename boost::mpl::find_if<Sequence, Contains<boost::mpl::_, T>>::type
												>>::type;
};


} // namespace details


template<typename T>
struct Contains<T, T> : std::true_type {};

template<typename ...Params, typename T>
struct Contains<Options<Params...>, T> : details::ContainsImp<std::tuple<Params...>, T>::type {};

}// namespace options

template <typename T, typename Options>
using IsByteType = typename boost::mpl::or_<std::is_same<T, std::uint8_t>, options::Contains<Options, bobl::options::ByteType<T>>>::type;

}/*namespace utility*/ } /*namespace bobl*/

