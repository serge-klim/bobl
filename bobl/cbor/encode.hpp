// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/cbor/details/encoder.hpp"
#include "bobl/options.hpp"
#include <vector>
#include <iterator>
#include <cstdint>
#include <cassert>


namespace bobl{ namespace cbor { 

template<typename ...Options, typename ...Args>
std::vector<std::uint8_t> encode(Args&& ...args) 
{
	std::vector<std::uint8_t> buffer;
	encoder::details::encode<bobl::Options<Options...>>(std::back_inserter(buffer), std::forward<Args>(args)...);
	return buffer;
}

template<typename ...Options, typename Iterator, typename ...Args>
auto encode(Iterator out, Args&& ...args) ->
	typename std::enable_if<std::is_same<typename std::iterator_traits<Iterator>::iterator_category, std::output_iterator_tag>::value, Iterator>::type
{
	return encoder::details::encode<bobl::Options<Options...>>(out, std::forward<Args>(args)...);
}

}/*namespace cbor*/ } /*namespace bobl*/

