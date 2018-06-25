#include "bobl/bson/decode.hpp"
#include "bobl/bson/encode.hpp"
#include "bobl/names.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <tuple>
#include <vector>
#include <cstdint>
#include <type_traits>
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

using SimpleTuple = std::tuple<bool, int, std::string, Enum>;

namespace bobl{

	template<typename MemberType, typename Options> class MemberName <SimpleTuple, MemberType, 0, Options>
	{
	public:
		constexpr char const* operator()() const { return "enabled"; }
	};

	template<typename Options> class MemberName <SimpleTuple, int, 1, Options>
	{
	public:
		constexpr char const* operator()() const { return "id"; }
	};

	template<std::size_t Position, typename Options> class MemberName <SimpleTuple, std::string, Position , Options>
	{
	public:
		constexpr char const* operator()() const { return "name"; }
	};

	template<typename MemberType, typename Options> class MemberName <SimpleTuple, MemberType, 3, Options>
	{
	public:
		constexpr char const* operator()() const { return "enm"; }
	};

}//namespace bobl


int main()
{
	auto tuple = std::make_tuple(true, 100, std::string{ "the name" }, Enum::Two);
	static_assert(std::is_same<SimpleTuple, decltype(tuple)>::value, "these types supposed to be the same");
	auto data = bobl::bson::encode(tuple);
	std::uint8_t const *begin = data.data();
	std::uint8_t const *end = begin + data.size();
	auto simple =  bobl::bson::decode<Simple>(begin, end);
	std::vector<std::uint8_t> data1 =  bobl::bson::encode(simple);

	{
		std::uint8_t const *begin = data1.data();
		std::uint8_t const *end = begin + data1.size();
		auto simple2 =  bobl::bson::decode<bool, int, std::string, Enum>(begin, end);
		assert(std::get<0>(tuple) == simple.enabled);
		assert(std::get<1>(tuple) == simple.id);
		assert(std::get<2>(tuple) == simple.name);
		assert(std::get<3>(tuple) == simple.enm);
	}

	return 0;
}
