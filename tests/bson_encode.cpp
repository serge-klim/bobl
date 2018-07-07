#include <boost/test/unit_test.hpp>
#include "bobl/bson/iterator.hpp"
#include "bobl/bson/cast.hpp"
#include "bobl/bson/decode.hpp"
#include "bobl/bson/flyweight.hpp"
#include "bobl/bson/encode.hpp"
#include "tests.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <tuple>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdint>



BOOST_AUTO_TEST_SUITE(BOBL_BSON_Encode_TestSuite)


BOOST_AUTO_TEST_CASE(EncodeToBufferTest)
{
	auto value = Simple{ true, 100, "the name", Two };
	std::vector<char> data;
	bobl::bson::encode(std::back_inserter(data), value);
	auto begin = data.data();
	auto end = begin + data.size();
	auto simple = bobl::bson::decode<Simple>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(EncodeTupleTest)
{
	auto value = std::make_tuple(true, 100, std::string{ "the name" }, TheEnumClass::One, TheEnumClass::Two);
	static_assert(!bobl::utility::NamedSequence<decltype(value), typename bobl::bson::EffectiveOptions<decltype(value), bobl::options::None>::type>::value, "not supposed to be named sequence");
	static_assert(bobl::utility::NamedSequence<decltype(value), typename bobl::bson::EffectiveOptions<decltype(value), bobl::options::UsePositionAsName>::type>::value, "supposed to be named sequence");
	auto data = bobl::bson::encode<bobl::options::UsePositionAsName>(value);
	auto begin = data.data();
	auto end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	using Any = bobl::flyweight::lite::Any<bobl::bson::flyweight::Iterator>;
	using NameValue = bobl::flyweight::NameValue<Any>;
	auto items = bobl::bson::make_iterator_range<NameValue>(doc);
	auto n = std::distance(items.begin(), items.end());
	BOOST_CHECK_EQUAL(n, decltype(n)(std::tuple_size<decltype(value)>::value));
	BOOST_CHECK_EQUAL(n, 5);
	auto i = items.begin();
	BOOST_CHECK_EQUAL(i->name(), std::string{"_0"});
	BOOST_CHECK_EQUAL(bobl::bson::cast<bool>(i->value()), true);
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{ "_1" });
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(i->value()), 100);
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{"_2"});
	BOOST_CHECK_EQUAL(bobl::bson::cast<std::string>(i->value()), std::string{ "the name" });
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{ "_3" });
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(i->value()), 1);
	BOOST_CHECK(bobl::bson::cast<TheEnumClass>(i->value()) == TheEnumClass::One);
	++i;
	BOOST_CHECK_EQUAL(i->name(), std::string{ "_4" });
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(i->value()), 2);
	BOOST_CHECK(bobl::bson::cast<TheEnumClass>(i->value()) == TheEnumClass::Two);
	{
		auto begin = data.data();
		auto end = begin + data.size();
		auto res = bobl::bson::decode<decltype(value), bobl::Options<bobl::options::UsePositionAsName>>(begin, end);
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
	auto data = bobl::bson::encode(value);
	auto begin = data.data();
	auto end = begin + data.size();
	auto simple = bobl::bson::decode<Simple>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}


BOOST_AUTO_TEST_CASE(OrderedIntDictionaryTest)
{
	auto value = OrderedIntDictionary{3, 128, 32767, 268435455, -4, -121, -32765, -268435454 };
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::bson::decode<OrderedIntDictionary, bobl::Options<bobl::options::RelaxedIntegers>>(begin, end);
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
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = bobl::bson::cast<Simple, bobl::Options<bobl::options::RelaxedIntegers>>(doc);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(WrappedEmptyArrayOfSimpleTest)
{
	auto value = Vector<Simple>{};
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<Vector<Simple>, bobl::Options<bobl::options::RelaxedIntegers>>(doc);
	BOOST_CHECK(res.vector.empty());
}

BOOST_AUTO_TEST_CASE(WrappeedArrayOfSimpleTest)
{
	auto value = Vector<Simple>{};
	value.vector.emplace_back(Simple{ true, 100, "null", None });
	value.vector.emplace_back(Simple{ false, 101, "first", One });
	value.vector.emplace_back(Simple{ true, 102, "second", Two });
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::bson::decode<Vector<Simple>, bobl::Options<bobl::options::RelaxedIntegers>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	//auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	//BOOST_CHECK_EQUAL(begin, end);
	//auto res = bobl::bson::cast<Vector<Simple>, bobl::Options<bobl::options::RelaxedIntegers>>(doc);
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

BOOST_AUTO_TEST_CASE(TupleAsIntArrayTest)
{
	auto value = std::make_tuple(0, 1, 2, 101);
	using ArrayType = decltype(value);
	auto data = bobl::bson::encode<bobl::options::UsePositionAsName, bobl::options::NonUniformArray<ArrayType>>(std::make_tuple(value));

	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<ArrayType, bobl::Options<bobl::options::NonUniformArray<ArrayType>>>(doc);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(std::get<0>(res), 0);
	BOOST_CHECK_EQUAL(std::get<1>(res), 1);
	BOOST_CHECK_EQUAL(std::get<2>(res), 2);
	BOOST_CHECK_EQUAL(std::get<3>(res), 101);
	auto v = bobl::bson::cast<std::vector<int>>(doc);
	BOOST_CHECK_EQUAL(v[0], 0);
	BOOST_CHECK_EQUAL(v[1], 1);
	BOOST_CHECK_EQUAL(v[2], 2);
	BOOST_CHECK_EQUAL(v[3], 101);

	using NameValue = bobl::flyweight::NameValue<std::vector<int>>;
	auto items = bobl::bson::make_iterator_range<NameValue>(doc);
	auto n = std::distance(items.begin(), items.end());
	BOOST_CHECK_EQUAL(n, 1);
	auto i = items.begin();
	BOOST_CHECK_EQUAL(i->value()[0], 0);
	BOOST_CHECK_EQUAL(i->value()[1], 1);
	BOOST_CHECK_EQUAL(i->value()[2], 2);
	BOOST_CHECK_EQUAL(i->value()[3], 101);
}


BOOST_AUTO_TEST_CASE(EmptyOptionalStructTest)
{
	auto value = SimpleOptional{};
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = bobl::bson::cast<SimpleOptional>(doc);
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
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = bobl::bson::cast<SimpleOptional>(doc);
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
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = bobl::bson::cast<SimpleVariant>(doc);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(boost::get<std::string>(simple.enabled).empty());
	//	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(boost::get<int>(simple.name), 0);
	BOOST_CHECK(boost::get<TheEnumClass>(simple.theEnum) == TheEnumClass::None);
}

BOOST_AUTO_TEST_CASE(OrderedFlotDictionaryTest, *boost::unit_test::tolerance(0.00001))
{
	auto value = OrderedFlotDictionary{0.0f, 3.333f, 65504.0f, 32767.0f, 2147483650.99999, -3.555f };
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<OrderedFlotDictionary>(doc);
	BOOST_CHECK_EQUAL(res.nul, 0);
	BOOST_TEST(res.short_ == 3.333);
	BOOST_CHECK_EQUAL(res.short_max, 65504.0);
	BOOST_CHECK_EQUAL(res.f32, 32767);
	BOOST_CHECK_EQUAL(res.f64, 2147483650.99999);
	BOOST_TEST(res.neg_short == -3.555);
}

BOOST_AUTO_TEST_CASE(ByteStringCharTest)
{
	auto value = Vector<char>{ std::vector<char>{ 100, 110, 120 } };
	auto data = bobl::bson::encode<bobl::Options<bobl::options::ByteType<char>>>(value);
	{
		uint8_t const* begin = data.data();
		uint8_t const* end = begin + data.size();
		BOOST_CHECK_THROW((bobl::bson::decode<Vector<char>>(begin, end)), bobl::IncorrectObjectType);
	}
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::bson::decode<Vector<char>, bobl::Options<bobl::options::ByteType<char>>>(begin, end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.vector), std::end(res.vector), std::begin(value.vector), std::end(value.vector));
}

BOOST_AUTO_TEST_CASE(ExtendedTest)
{
	auto value = Extended{ 101, Simple{ false, 303, "first", One }, std::vector<std::uint8_t>{100, 110, 120}, 973 };
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::bson::decode<Extended>(begin, end);
	BOOST_CHECK_EQUAL(res.id, 101);
	BOOST_CHECK_EQUAL(res.simple.id, 303);
	BOOST_CHECK_EQUAL(res.simple.name, std::string{ "first" });
	BOOST_CHECK_EQUAL(int(res.simple.theEnum), 1);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.bin), std::end(res.bin), std::begin(value.bin), std::end(value.bin));
	BOOST_CHECK_EQUAL(res.term, 973);
}

BOOST_AUTO_TEST_CASE(ByteStringCharAsAnyTest)
{
	auto value = Vector<std::uint8_t>{ std::vector<std::uint8_t>{ 100, 110, 120 } };
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto any = bobl::bson::decode<bobl::flyweight::lite::Any<decltype(begin)>>(begin, end);
	BOOST_CHECK_THROW((bobl::bson::cast<Vector<char>>(any)), bobl::IncorrectObjectType);
	auto res = bobl::bson::cast<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(any);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(value.vector), std::end(value.vector));
}

BOOST_AUTO_TEST_CASE(UuidTest)
{
	boost::uuids::string_generator gen;
	auto uuids = std::vector<boost::uuids::uuid>{ gen("4E983010-FA64-4BAA-ABED-DD82FD691D18"), gen("97B9B488-3B45-4C50-8144-719A8414216C") };
	auto value = Vector<boost::uuids::uuid>{ std::move(uuids) };
	auto data = bobl::bson::encode(value);
	{
		uint8_t const* begin = data.data();
		uint8_t const* end = begin + data.size();
		BOOST_CHECK_THROW((bobl::bson::decode<Vector<char>>(begin, end)), bobl::IncorrectObjectType);
	}
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::bson::decode<Vector<boost::uuids::uuid>>(begin, end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.vector), std::end(res.vector), std::begin(value.vector), std::end(value.vector));
}

BOOST_AUTO_TEST_CASE(TimepointTest)
{
	auto tps = std::vector<std::chrono::system_clock::time_point>{ std::chrono::system_clock::now() };
	auto value = Vector<std::chrono::system_clock::time_point>{ std::move(tps) };
	auto data = bobl::bson::encode(value);
	{
		uint8_t const* begin = data.data();
		uint8_t const* end = begin + data.size();
		BOOST_CHECK_THROW((bobl::bson::decode<Vector<char>>(begin, end)), bobl::IncorrectObjectType);
	}
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto res = bobl::bson::decode<Vector<std::chrono::system_clock::time_point>>(begin, end);
	BOOST_CHECK_EQUAL(res.vector.size(), 1);
	BOOST_CHECK((res.vector[0] - value.vector[0] < std::chrono::milliseconds{1}));
	BOOST_CHECK((value.vector[0] - res.vector[0] < std::chrono::milliseconds{ 1 }));
}

BOOST_AUTO_TEST_CASE(NamedVariantTest)
{
	using NamedVariant = diversion::variant<bobl::UseTypeName, Simple, Extended>;
	auto value = Simple{ true, 100, "the name", Two };
 	auto data = bobl::bson::encode(NamedVariant{ value });
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto variant = bobl::bson::cast<NamedVariant>(doc);
	auto simple = boost::get<Simple>(variant);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(FewNamedVariantTest)
{
	using NamedVariant = diversion::variant<bobl::UseTypeName, Simple, Extended>;
	auto value1 = Simple{ true, 100, "the name", Two };
	auto value2 = Extended{ 303, Simple{ false, 707, "another name", Three }, {0, 1, 2}, 999 };
	auto data = bobl::bson::encode(NamedVariant{ value1 }, NamedVariant{ value2 });
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<std::tuple<NamedVariant, NamedVariant>>(doc);
	auto variant1 = std::get<0>(res);
	auto simple = boost::get<Simple>(variant1);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
	auto variant2 = std::get<1>(res);
	auto extended = boost::get<Extended>(variant2);
	BOOST_CHECK_EQUAL(extended.id, 303);
	BOOST_CHECK_EQUAL(extended.simple.enabled, false);
	BOOST_CHECK_EQUAL(extended.simple.id, 707);
	BOOST_CHECK_EQUAL(extended.simple.name, std::string{ "another name" });
	BOOST_CHECK_EQUAL(int(extended.simple.theEnum), 3);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(extended.bin), std::end(extended.bin), std::begin(value2.bin), std::end(value2.bin));
	BOOST_CHECK_EQUAL(extended.term, 999);
}

BOOST_AUTO_TEST_CASE(NamedVariantAsVariantTest)
{
	auto value = NamedVariant{ Simple{ true, 100, "the name", Two }, 301 };
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<NamedVariant>(doc);
	auto const& simple = boost::get<Simple>(res.named);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
	BOOST_CHECK_EQUAL(res.dummy, value.dummy);
}

BOOST_AUTO_TEST_CASE(OptionalTest)
{
	auto data = SimpleOptionalTest{ {}, 123 };
	std::vector<std::uint8_t> encoded = bobl::bson::encode(data);
	auto begin = encoded.data();
	auto end = begin + encoded.size();
	auto decoded = bobl::bson::decode<SimpleOptionalTest>(begin, end);
	BOOST_CHECK(!decoded.type);
	BOOST_CHECK_EQUAL(decoded.id, data.id);

	begin = encoded.data();
	auto decoded_tuple = bobl::bson::decode<boost::optional<TheEnum>, boost::optional<int>>(begin, end);
	//this supposed to be broken please see README.md
	BOOST_CHECK_EQUAL(int(std::get<0>(decoded_tuple).get()), 123);
	BOOST_CHECK(!std::get<1>(decoded_tuple));

	begin = encoded.data();
	BOOST_CHECK_THROW((bobl::bson::decode<diversion::optional<TheEnumClass>, int>(begin, end)), bobl::InvalidObject);
}

BOOST_AUTO_TEST_SUITE_END()
