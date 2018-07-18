// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/options.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/bobl.hpp"
#include <boost/mpl/or.hpp>
#include <type_traits>

namespace bobl{ namespace utility {

template<typename T, typename Options>
struct VariantUseTypeName : std::false_type {};

template<typename ...Types, typename Options>
struct VariantUseTypeName<diversion::variant<Types...>, Options>  : 
				 boost::mpl::or_<
								bobl::utility::options::Contains<Options, bobl::options::UseTypeName<diversion::variant<Types...>>>,
								options::details::ContainsImp<std::tuple<Types...>, bobl::UseTypeName>>::type
{};

}/*namespace utility*/ } /*namespace bobl*/

