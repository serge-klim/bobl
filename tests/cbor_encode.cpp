#include <boost/test/unit_test.hpp>
#include "bobl/cbor/cast.hpp"
#include "bobl/cbor/iterator.hpp"
#include "bobl/cbor/encode.hpp"
#include "bobl/cbor/decode.hpp"
#include "tests.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <tuple>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdint>


BOOST_AUTO_TEST_SUITE(BOBL_CBOR_Encode_TestSuite)

BOOST_AUTO_TEST_CASE(EncodeToBufferTest)
{
	auto value = Simple{ true, 100, "the name", Two };
	std::vector<char> data;
	bobl::cbor::encode(std::back_inserter(data), value);
	auto begin = data.data();
	auto end = begin + data.size();
	auto simple = bobl::cbor::decode<Simple>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(EncodeTupleTest)
{
	auto value = std::make_tuple(true, 100, std::string{ "the name" }, TheEnumClass::One, TheEnumClass::Two);
	static_assert(!bobl::utility::NamedSequence<decltype(value), typename bobl::cbor::EffectiveOptions<decltype(value), bobl::options::None>::type>::value, "not supposed to be named sequence");
	static_assert(bobl::utility::NamedSequence<decltype(value), typename bobl::cbor::EffectiveOptions<decltype(value), bobl::options::UsePositionAsName>::type>::value, "supposed to be named sequence");
	auto data = bobl::cbor::encode<bobl::options::UsePositionAsName>(value);
	auto begin = data.data();
	auto end = begin + data.size();
	auto object = bobl::cbor::decode<bobl::flyweight::lite::Object<decltype(begin)>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	using NameValue = bobl::flyweight::NameValue<bobl::flyweight::lite::Any<decltype(begin)>>;
	auto items = bobl::cbor::make_iterator_range<NameValue>(object);
	auto n = std::distance(items.begin(), items.end());
	BOOST_CHECK_EQUAL(n, decltype(n)(std::tuple_size<decltype(value)>::value));
	BOOST_CHECK_EQUAL(n, 5);
	auto i = items.begin();
	BOOST_CHECK_EQUAL(i->name(), std::string{"_0"});
	BOOST_CHECK_EQUAL(bobl::cbor::cast<bool>(i->value()), true);
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{ "_1" });
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(i->value()), 100);
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{"_2"});
	BOOST_CHECK_EQUAL(bobl::cbor::cast<std::string>(i->value()), std::string{ "the name" });
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{ "_3" });
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(i->value()), 1);
	BOOST_CHECK(bobl::cbor::cast<TheEnumClass>(i->value()) == TheEnumClass::One);
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{ "_4" });
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(i->value()), 2);
	BOOST_CHECK(bobl::cbor::cast<TheEnumClass>(i->value()) == TheEnumClass::Two);
	{
		auto begin = data.data();
		auto end = begin + data.size();
		auto res = bobl::cbor::decode<decltype(value), bobl::Options<bobl::options::UsePositionAsName>>(begin, end);
		BOOST_CHECK_EQUAL(begin, end);
		BOOST_CHECK_EQUAL(std::get<0>(res), std::get<0>(value));
		BOOST_CHECK_EQUAL(std::get<1>(res), std::get<1>(value));
		BOOST_CHECK_EQUAL(std::get<2>(res), std::get<2>(value));
		BOOST_CHECK(std::get<3>(res) == std::get<3>(value));
		BOOST_CHECK(std::get<4>(res) == std::get<4>(value));
	}
}

BOOST_AUTO_TEST_CASE(EncodeNamedTupleTest)
{
	auto value = std::make_tuple(true, 100, std::string{ "the name" }, TheEnumClass::Two);
	static_assert(std::is_same<SimpleTuple, decltype(value)>::value, "these types supposed to be the same");
	auto data = bobl::cbor::encode(value);
	auto begin = data.data();
	auto end = begin + data.size();
	auto simple = bobl::cbor::decode<Simple>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(OrderedIntDictionaryTest)
{
	auto value = OrderedIntDictionary{ 3, 128, 32767, 268435455, -4, -121, -32765, -268435454 };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<OrderedIntDictionary>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
	BOOST_CHECK_EQUAL(res.int_, 32767);
	BOOST_CHECK_EQUAL(res.int64, 268435455);
	BOOST_CHECK_EQUAL(res.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.neg_short, -121);
	BOOST_CHECK_EQUAL(res.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
}

BOOST_AUTO_TEST_CASE(SimpleTest)
{
	auto value = Simple{ true, 100, "the name", Two };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto simple = bobl::cbor::decode<Simple>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(EmptyArrayOfSimpleTest)
{
	auto value = std::vector<Simple>{};
	//auto value = Simple{ true, 100, "the name", Two };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<std::vector<Simple>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(res.empty());
}

BOOST_AUTO_TEST_CASE(ArrayOfSimpleTest)
{
	auto value = std::vector<Simple>{};
	value.emplace_back(Simple{ true, 100, "null", None });
	value.emplace_back(Simple{ false, 101, "first", One });
	value.emplace_back(Simple{ true, 102, "second", Two });
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<std::vector<Simple>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.size(), 3);
	BOOST_CHECK(res[0].enabled);
	BOOST_CHECK_EQUAL(res[0].id, 100);
	BOOST_CHECK_EQUAL(res[0].name, std::string{ "null" });
	BOOST_CHECK_EQUAL(int(res[0].theEnum), 0);
	BOOST_CHECK(!res[1].enabled);
	BOOST_CHECK_EQUAL(res[1].id, 101);
	BOOST_CHECK_EQUAL(res[1].name, std::string{ "first" });
	BOOST_CHECK_EQUAL(int(res[1].theEnum), 1);
	BOOST_CHECK(res[2].enabled);
	BOOST_CHECK_EQUAL(res[2].id, 102);
	BOOST_CHECK_EQUAL(res[2].name, std::string{ "second" });
	BOOST_CHECK_EQUAL(int(res[2].theEnum), 2);
}

BOOST_AUTO_TEST_CASE(WrappedEmptyArrayOfSimpleTest)
{
	auto value = Vector<Simple>{};
	//auto value = Simple{ true, 100, "the name", Two };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<Vector<Simple>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(res.vector.empty());
}

BOOST_AUTO_TEST_CASE(WrappeedArrayOfSimpleTest)
{
	auto value = Vector<Simple>{};
	value.vector.emplace_back(Simple{ true, 100, "null", None });
	value.vector.emplace_back(Simple{ false, 101, "first", One });
	value.vector.emplace_back(Simple{ true, 102, "second", Two });
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<Vector<Simple>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.vector.size(), 3);
	BOOST_CHECK(res.vector[0].enabled);
	BOOST_CHECK_EQUAL(res.vector[0].id, 100);
	BOOST_CHECK_EQUAL(res.vector[0].name, std::string{ "null" });
	BOOST_CHECK_EQUAL(int(res.vector[0].theEnum), 0);
	BOOST_CHECK(!res.vector[1].enabled);
	BOOST_CHECK_EQUAL(res.vector[1].id, 101);
	BOOST_CHECK_EQUAL(res.vector[1].name, std::string{ "first" });
	BOOST_CHECK_EQUAL(int(res.vector[1].theEnum), 1);
	BOOST_CHECK(res.vector[2].enabled);
	BOOST_CHECK_EQUAL(res.vector[2].id, 102);
	BOOST_CHECK_EQUAL(res.vector[2].name, std::string{ "second" });
	BOOST_CHECK_EQUAL(int(res.vector[2].theEnum), 2);
}

BOOST_AUTO_TEST_CASE(EmptyOptionalStructTest)
{
	auto value = SimpleOptional{};
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto simple = bobl::cbor::decode<SimpleOptional>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(!simple.enabled);
	BOOST_CHECK(!simple.id);
	BOOST_CHECK(!simple.name);
	BOOST_CHECK(!simple.theEnum);
	BOOST_CHECK(!simple.dummy1);
	BOOST_CHECK(!simple.dummy2);
	BOOST_CHECK(!simple.dummy3);
}

BOOST_AUTO_TEST_CASE(OptionalStructTest)
{
	auto value = SimpleOptional{};
	value.id = 101;
	value.name = "the name";
	value.theEnum = TheEnumClass::Two;
	auto strings = std::vector<std::string>{"zero","one", "two", "three"};
	value.dummy2 = strings;
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto simple = bobl::cbor::decode<SimpleOptional>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(!simple.enabled);
	BOOST_CHECK_EQUAL(!simple.id,false);
	BOOST_CHECK_EQUAL(simple.id.get(), value.id.get());
	BOOST_CHECK_EQUAL(!simple.name, false);
	BOOST_CHECK_EQUAL(simple.name.get(), value.name.get());
	BOOST_CHECK_EQUAL(!simple.theEnum, false);
	BOOST_CHECK(simple.theEnum.get() == value.theEnum.get());
	BOOST_CHECK(!simple.dummy1);
	BOOST_CHECK_EQUAL(!simple.dummy2, false);
	{
		BOOST_CHECK_EQUAL(!simple.dummy2, false);
		auto const& strings = simple.dummy2.get();
		BOOST_CHECK_EQUAL(strings.size(), 4);
		BOOST_CHECK_EQUAL(strings[1], std::string{ "one" });
	}
	BOOST_CHECK(!simple.dummy3);
}

BOOST_AUTO_TEST_CASE(EmptySimpleXTest)
{
	auto value = SimpleVariant{ /*true, 100, "the name", Two*/ };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto simple = bobl::cbor::decode<SimpleVariant>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(boost::get<std::string>(simple.enabled).empty());
//	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(boost::get<int>(simple.name), 0);
	BOOST_CHECK(boost::get<TheEnumClass>(simple.theEnum) == TheEnumClass::None);
}


BOOST_AUTO_TEST_CASE(OrderedFlotDictionaryTest, *boost::unit_test::tolerance(0.00001))
{
	auto value = OrderedFlotDictionary{ 0.0f, 3.333f, 65504.0f, 32767.0f, 2147483650.99999, -3.555f };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<OrderedFlotDictionary>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.nul, 0);
	BOOST_TEST(res.short_ == 3.333);
	BOOST_CHECK_EQUAL(res.short_max, 65504.0);
	BOOST_CHECK_EQUAL(res.f32, 32767);
	BOOST_CHECK_EQUAL(res.f64, 2147483650.99999);
	BOOST_TEST(res.neg_short == -3.555);
}


BOOST_AUTO_TEST_CASE(ByteStringTest)
{
	auto value = std::vector<std::uint8_t>{100, 110, 120};
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<std::vector<std::uint8_t>>(begin, end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(value), std::end(value));
}

BOOST_AUTO_TEST_CASE(ByteStringCharTest)
{
	auto value = std::vector<char>{ 100, 110, 120 };
	auto data = bobl::cbor::encode<bobl::options::ByteType<char>>(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(begin, end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(value), std::end(value));
}

BOOST_AUTO_TEST_CASE(ExtendedTest)
{
	auto value = Extended{ 101, Simple{ false, 303, "first", One }, std::vector<std::uint8_t>{100, 110, 120}, 973 };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<Extended>(begin, end);
	BOOST_CHECK_EQUAL(res.id, 101);
	BOOST_CHECK_EQUAL(res.simple.id, 303);
	BOOST_CHECK_EQUAL(res.simple.name, std::string{ "first" });
	BOOST_CHECK_EQUAL(int(res.simple.theEnum), 1);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.bin), std::end(res.bin), std::begin(value.bin), std::end(value.bin));
	BOOST_CHECK_EQUAL(res.term, 973);
}


BOOST_AUTO_TEST_CASE(ByteStringCharAsAnyTest)
{
	auto value = std::vector<std::uint8_t>{ 100, 110, 120 };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto any = bobl::cbor::decode<bobl::flyweight::lite::Any<decltype(begin)>>(begin, end);
	BOOST_CHECK_THROW((bobl::cbor::cast<std::vector<char>>(any)), bobl::IncorrectObjectType);
	bobl::cbor::cast<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(any);
	auto res = bobl::cbor::cast<std::vector<std::uint8_t>>(any);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(value), std::end(value));
}


BOOST_AUTO_TEST_CASE(UuidTest)
{
	boost::uuids::string_generator gen;
	auto uuids = std::vector<boost::uuids::uuid>{ gen("4E983010-FA64-4BAA-ABED-DD82FD691D18"), gen("97B9B488-3B45-4C50-8144-719A8414216C") };
	auto value = Vector<boost::uuids::uuid>{ std::move(uuids) };
	auto data = bobl::cbor::encode(value);
	{
		uint8_t const* begin = data.data();
		uint8_t const* end = begin + data.size();
		BOOST_CHECK_THROW((bobl::cbor::decode<Vector<char>>(begin, end)), bobl::IncorrectObjectType);
	}
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<Vector<boost::uuids::uuid>>(begin, end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.vector), std::end(res.vector), std::begin(value.vector), std::end(value.vector));
}

BOOST_AUTO_TEST_CASE(TimepointTest)
{
	auto tps = std::vector<std::chrono::system_clock::time_point>{ std::chrono::system_clock::now() };
	auto value = Vector<std::chrono::system_clock::time_point>{ std::move(tps) };
	auto data = bobl::cbor::encode(value);
	{
		uint8_t const* begin = data.data();
		uint8_t const* end = begin + data.size();
		BOOST_CHECK_THROW((bobl::cbor::decode<Vector<char>>(begin, end)), bobl::IncorrectObjectType);
	}
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<Vector<std::chrono::system_clock::time_point>>(begin, end);
	BOOST_CHECK_EQUAL(res.vector.size(), 1);
	BOOST_CHECK((res.vector[0] - value.vector[0] < std::chrono::seconds{ 1 }));
	BOOST_CHECK((value.vector[0] - res.vector[0] < std::chrono::seconds{ 1 }));
}

BOOST_AUTO_TEST_CASE(NamedVariantTest)
{
	using NamedVariant = diversion::variant<bobl::UseTypeName, Simple, Extended>;
	auto value = Simple{ true, 100, "the name", Two };
	//auto data = bobl::cbor::encode(bobl::TypeName<decltype(value)>{}(), value);
	auto data = bobl::cbor::encode(NamedVariant{ value });
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto variant = bobl::cbor::decode<NamedVariant>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = boost::get<Simple>(variant);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(NamedVariantAsVariantTest)
{
	auto value = NamedVariant{ Simple{ true, 100, "the name", Two }, 301 };
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::cbor::decode<NamedVariant>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto const& simple = boost::get<Simple>(res.named);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
	BOOST_CHECK_EQUAL(res.dummy, value.dummy);
}

BOOST_AUTO_TEST_SUITE_END()
