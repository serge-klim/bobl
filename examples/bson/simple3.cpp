#include "bobl/bson/decode.hpp"
#include "bobl/bson/encode.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <tuple>
#include <vector>
#include <cstdint>
#include <cassert>


enum class Enum { None, One, Two, Three };

struct Simple
{
	bool enabled;
    int id;
	std::string name;
	Enum enm;
};

BOOST_FUSION_ADAPT_STRUCT(
	Simple,
	enabled,
    id,
	name,
	enm)


int main()
{
	std::uint8_t data[] = {0x33, 0x0,  0x0,  0x0,  0x8, 0x65, 0x6e, 0x61, 0x62,
						   0x6c, 0x65, 0x64, 0x0,  0x1, 0x10, 0x69, 0x64, 0x0,
						   0x64, 0x0,  0x0,  0x0,  0x2, 0x6e, 0x61, 0x6d, 0x65,
						   0x0,  0x9,  0x0,  0x0,  0x0, 0x74, 0x68, 0x65, 0x20,
						   0x6e, 0x61, 0x6d, 0x65, 0x0, 0x10, 0x65, 0x6e, 0x6d,
						   0x0,  0x2,  0x0,  0x0,  0x0, 0x0};

	std::uint8_t const *begin = data;
	std::uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto simple =  bobl::bson::decode<Simple>(begin, end);
	std::vector<std::uint8_t> data1 =  bobl::bson::encode(simple);

	{
		std::uint8_t const *begin = data1.data();
		std::uint8_t const *end = begin + data1.size();
		auto simple2 =  bobl::bson::decode<bool, int, std::string, Enum>(begin, end);
		assert(std::get<0>(simple2) == simple.enabled);
		assert(std::get<1>(simple2) == simple.id);
		assert(std::get<2>(simple2) == simple.name);
		assert(std::get<3>(simple2) == simple.enm);
	}

	return 0;
}
