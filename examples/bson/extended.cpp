#include "bobl/bson/decode.hpp"
#include "bobl/bson/encode.hpp"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <chrono>
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

struct Extended
{
	int id;
	Simple simple;
	std::vector<int> ints;
	std::vector<Simple> simples;
	boost::variant<int,Simple, std::string, std::vector<Simple>> var;
	boost::uuids::uuid uuid;
	boost::optional<Enum> enm;
	std::vector<std::uint8_t> binary; // this will be encoded as binary object
	std::chrono::system_clock::time_point tp;
};

BOOST_FUSION_ADAPT_STRUCT(
	Extended,
	id,
	simple,
	ints,
	simples,
	var,
	uuid,
	enm,
	binary,
	tp)

int main()
{
	boost::uuids::string_generator gen;
	auto extended = Extended{ 1,
							  {false, 303, "the name", Enum::One },
							  {1,2,3},
							  {
								  {true, 1, "first", Enum::One },
								  {false, 2, "second", Enum::Two },
							  },
							  101,
							  gen("4E983010-FA64-4BAA-ABED-DD82FD691D18"),
							  Enum::Three,
							  {0x1,0x2, 0x3},
							  std::chrono::system_clock::now()
	};
	std::vector<std::uint8_t> encoded =  bobl::bson::encode(extended);
    auto begin = encoded.data();
	auto end = begin + encoded.size();
	auto extended2 =  bobl::bson::decode<Extended>(begin, end);
	return 0;
}
