#include "bobl/cbor/decode.hpp"
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

  std::uint8_t data[] = {0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64,
                         0xf5, 0x62, 0x69, 0x64, 0x18, 0x64, 0x64, 0x6e, 0x61,
                         0x6d, 0x65, 0x68, 0x74, 0x68, 0x65, 0x20, 0x6e, 0x61,
                         0x6d, 0x65, 0x63, 0x65, 0x6e, 0x6d, 0x2};

  uint8_t const *begin = data;
  uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
  auto res =  bobl::cbor::decode<Simple>(begin, end);

  return 0;
}
