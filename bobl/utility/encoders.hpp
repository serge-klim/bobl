// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/utils.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/diversion.hpp"
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/count_if.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/placeholders.hpp>
#include <type_traits>

namespace bobl{ namespace utility{

template<typename NsTag, typename T, typename Options>
struct OptionalyEncoded : std::false_type {};

template<typename NsTag, typename T, typename Options>
struct OptionalyEncoded<NsTag, diversion::optional<T>, Options> : boost::mpl::not_<bobl::utility::options::Contains<typename bobl::EffectiveOptions<NsTag, T, Options>::type, bobl::options::OptionalAsNull>>::type {};

template<typename NsTag, typename Sequence, typename Options, typename HasOptional = std::true_type>
struct CountMembers
{
	static_assert(boost::fusion::traits::is_sequence<Sequence>::value, "CountMembers requires boost mpl sequence");
	struct HasValue
	{
		template<typename T>
		constexpr bool operator() (T const& /*val*/) const { return true; }

		template<typename T>
		constexpr bool operator() (diversion::optional<T> const& val) const { return !val == false; }
	};

	size_t operator()(Sequence const& sequence) const {	return boost::fusion::count_if(sequence, HasValue{}); }
};

template<typename NsTag, typename Sequence, typename Options>
struct CountMembers<NsTag, Sequence, Options,
							typename std::is_same<typename boost::mpl::end<Sequence>::type,
																	typename boost::mpl::find_if<Sequence, OptionalyEncoded<NsTag, boost::mpl::_, Options>>::type
																	>::type>
{
	static_assert(boost::fusion::traits::is_sequence<Sequence>::value, "CountMembers requires boost mpl sequence");
	constexpr size_t operator()(Sequence const&) const { return boost::fusion::result_of::size<Sequence>::value; }
};

}/*namespace bobl*/} /*namespace utility*/


