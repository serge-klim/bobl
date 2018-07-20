#pragma once
#include "bobl/utility/diversion.hpp"
#include "bobl/names.hpp"
#include "bobl/bobl.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/cstdfloat.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

enum TheEnum
{
	None, One, Two, Three
};

enum class TheEnumClass
{
    None, One, Two, Three
};

struct Empty{};

BOOST_FUSION_ADAPT_STRUCT(
    Empty)

struct Simple
{
	bool enabled;
    int id;
	std::string name;
	TheEnum theEnum;
};

BOOST_FUSION_ADAPT_STRUCT(
	Simple,
	enabled,
	id,
	name,
	theEnum)

using SimpleTuple = std::tuple<bool, int, std::string, TheEnumClass>;

namespace bobl{

template<typename MemberType, typename Options> class MemberName <SimpleTuple, MemberType, 0, Options>
{
public:
	constexpr char const* operator()() const { return "enabled"; }
};

template<typename MemberType, typename Options> class MemberName <SimpleTuple, MemberType, 1, Options>
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
	constexpr char const* operator()() const { return "theEnum"; }
};

}//namespace bobl

struct SimpleOptional
{
	diversion::optional<bool> enabled;
	diversion::optional<int> id;
	diversion::optional<std::string> name;
	diversion::optional<TheEnumClass> theEnum;
	diversion::optional<std::string> dummy1;
	diversion::optional<std::vector<std::string>> dummy2;
	diversion::optional<std::vector<Simple>> dummy3;
};

BOOST_FUSION_ADAPT_STRUCT(
	SimpleOptional,
	enabled,
	id,
	name,
	theEnum,
	dummy1,
	dummy2,
	dummy3)

struct SimpleVariant
{
	diversion::variant<std::string, int, bool> enabled;
    int id;
	diversion::variant<int, TheEnum, std::string> name;
	diversion::variant<TheEnumClass> theEnum;
};

BOOST_FUSION_ADAPT_STRUCT(
	SimpleVariant,
	enabled,
    id,
	name,
	theEnum)

struct IntDictionary
{
	//{'tiny': 3, 'short': 128, 'int': 32767, 'int64': 268435455, 'neg-tiny': -4, 'neg-short': -121, 'neg-int': -32765, 'neg-int64': -268435454}
	short short_;
	std::int64_t neg_int64;
	unsigned short tiny;
	std::int64_t int64;
	short neg_tiny;
	int int_;
	short neg_short;
	int neg_int;
};

BOOST_FUSION_ADAPT_STRUCT(
	IntDictionary,
	short_,
	neg_int64,
	tiny,
	int64,
	neg_tiny,
	int_, 
	neg_short,
	neg_int,
)

struct IntDictionaryX
{
	std::string xtra1;
	//{'tiny': 3, 'short': 128, 'int': 32767, 'int64': 268435455, 'neg-tiny': -4, 'neg-short': -121, 'neg-int': -32765, 'neg-int64': -268435454}
	short short_;
	std::int64_t neg_int64;
	unsigned short tiny;
	std::int64_t int64;
	std::string xtra2;
	short neg_tiny;
	int int_;
	short neg_short;
	int neg_int;
	std::string xtra3;
};

BOOST_FUSION_ADAPT_STRUCT(
	IntDictionaryX,
	xtra1,
	short_,
	neg_int64,
	tiny,
	int64,
	xtra2,
	neg_tiny,
	int_, 
	neg_short,
	neg_int,
	xtra3
)

struct OrderedIntDictionary
{
	//{'tiny': 3, 'short': 128, 'int': 32767, 'int64': 268435455, 'neg-tiny': -4, 'neg-short': -121, 'neg-int': -32765, 'neg-int64': -268435454}
	unsigned short tiny;
	short short_;
	int int_;
	std::int64_t int64;
	short neg_tiny;
	short neg_short;
	int neg_int;
	std::int64_t neg_int64;
};

BOOST_FUSION_ADAPT_STRUCT(
	OrderedIntDictionary,
	tiny,
	short_,
	int_,
	int64,
	neg_tiny,
	neg_short,
	neg_int,
	neg_int64,
	)

struct OrderedIntDictionary1
{
	//{'tiny': 3, 'short': 128, 'int': 32767, 'int64': 268435455, 'neg-tiny': -4, 'neg-short': -121, 'neg-int': -32765, 'neg-int64': -268435454}
	unsigned short tiny;
	short short_;
	int int_;
	std::int64_t int64;
};

BOOST_FUSION_ADAPT_STRUCT(
	OrderedIntDictionary1,
	tiny,
	short_,
	int_,
	int64,
	)

struct ShortOrderedIntDictionary
{
	//{'tiny': 3, 'short': 128, 'int': 32767, 'int64': 268435455, 'neg-tiny': -4, 'neg-short': -121, 'neg-int': -32765, 'neg-int64': -268435454}
	unsigned short tiny;
	short short_;
};

BOOST_FUSION_ADAPT_STRUCT(
	ShortOrderedIntDictionary,
	tiny,
	short_,
	)

struct OrderedFlotDictionary
{
	//{'nul': 0.0, 'short_': 3.333, 'short_max': 65504.0, 'f32': 32767, 'f64': 2147483650.99999, 'neg_short': -3.555}
	float nul;
	float short_;
	float short_max;
	float f32;
	double f64;
	float neg_short;
};

BOOST_FUSION_ADAPT_STRUCT(
	OrderedFlotDictionary,
	 nul,
	 short_,
	 short_max,
	 f32,
	 f64,
	 neg_short,
	)

namespace levels {
	struct Level3
	{
		std::string name;
		short short_;
	};

	struct Level2
	{
		std::string name;
		short short_;
		Level3 level3;
	};

	struct Level1
	{
		std::string name;
		short short_;
		Level2 level2;
	};
} //namespace leves

BOOST_FUSION_ADAPT_STRUCT(
	levels::Level3,
	name,
	short_)

BOOST_FUSION_ADAPT_STRUCT(
	levels::Level2,
	name,
	short_,
	level3)

BOOST_FUSION_ADAPT_STRUCT(
	levels::Level1,
	name,
	short_,
	level2)

template<typename Ints = OrderedIntDictionary>
struct Nested
{
	std::string name;
	Ints ints;
	levels::Level1 level1;
};

BOOST_FUSION_ADAPT_TPL_STRUCT(
	(Ints),
	(Nested)(Ints),
	name,
	ints,
	level1
)

template<typename T>
struct Vector
{
	std::vector<T> vector;
};

BOOST_FUSION_ADAPT_TPL_STRUCT(
	(T),
	(Vector)(T),
	vector
)

struct DisabledNames
{
	int enabled;
	int disabled;
};

BOOST_FUSION_ADAPT_STRUCT(
	DisabledNames,
	enabled,
	disabled)


struct Extended
{
	int id;
	Simple simple;
	std::vector<std::uint8_t> bin;
	int term;
};

BOOST_FUSION_ADAPT_STRUCT(
	Extended,
	id,
	simple,
	bin,
	term)

struct NamedVariant
{
	diversion::variant<bobl::UseTypeName, Simple, Extended> named;
	int dummy;
};

BOOST_FUSION_ADAPT_STRUCT(
	NamedVariant,
	named,
	dummy)

struct SimpleOptionalTest
{
	diversion::optional<TheEnumClass> type; //will be encoded as int
	int id;
};

BOOST_FUSION_ADAPT_STRUCT(SimpleOptionalTest, type, id)


class CheckMessage
{
public:
	CheckMessage(std::string msg) : msg_{ std::move(msg) } {}
	bool operator()(std::exception const& e) const
	{
		return msg_.compare(e.what()) == 0;
	}
private:
	std::string msg_;
};
