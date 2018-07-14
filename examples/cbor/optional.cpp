#include "bobl/cbor/decode.hpp"
#include "bobl/cbor/encode.hpp"
#include <boost/optional.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <tuple>
#include <cstdint>
#include <cassert>


enum class Type { One, Two, Three };

struct Data
{
	boost::optional<Type> type; //will be encoded as int
	int id;
};

BOOST_FUSION_ADAPT_STRUCT(Data, type, id)


int main()
{
	auto data  = Data { {}, 123};
	std::vector<std::uint8_t> encoded =  bobl::cbor::encode(data);
    auto begin = encoded.data();
	auto end = begin + encoded.size();
//this will work as expected
	auto decoded =  bobl::cbor::decode<Data>(begin, end);
	assert(!decoded.type);
	assert(decoded.id == data.id);

	begin = encoded.data();
	auto decoded_tuple = bobl::cbor::decode<boost::optional<Type>, boost::optional<int>>(begin, end);
	//this supposed to be broken please see README.md
	assert(int(std::get<0>(decoded_tuple).get()) == 123);
	assert(!std::get<1>(decoded_tuple));
	try
	{
		auto begin = encoded.data();
		bobl::cbor::decode<boost::optional<Type>, int>(begin, end);
		assert(!"it should throw before it gets here");
	}catch(bobl::InputToShort&)
	{
	}

	return 0;
}
