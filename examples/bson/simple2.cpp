#include "bobl/bson/decode.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

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


int main() {

  std::uint8_t data[] = {0x33, 0x0,  0x0,  0x0,  0x8, 0x65, 0x6e, 0x61, 0x62,
                         0x6c, 0x65, 0x64, 0x0,  0x1, 0x10, 0x69, 0x64, 0x0,
                         0x64, 0x0,  0x0,  0x0,  0x2, 0x6e, 0x61, 0x6d, 0x65,
                         0x0,  0x9,  0x0,  0x0,  0x0, 0x74, 0x68, 0x65, 0x20,
                         0x6e, 0x61, 0x6d, 0x65, 0x0, 0x10, 0x65, 0x6e, 0x6d,
                         0x0,  0x2,  0x0,  0x0,  0x0, 0x0};

  uint8_t const *begin = data;
  uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
  auto res =  bobl::bson::decode<Simple>(begin, end);

  return 0;
}