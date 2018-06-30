#include <boost/test/unit_test.hpp>

#include "bobl/utility/iterator.hpp"
#include "bobl/bson/cast.hpp"
#include "bobl/bson/flyweight.hpp"
#include "bobl/bson/decode.hpp"
#include "bobl/utility/decoders.hpp"
#include "tests.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <tuple>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdint>


BOOST_AUTO_TEST_SUITE(BOBL_BSON_TestSuite)


BOOST_AUTO_TEST_CASE(RawTest)
{
	//{'id': 'true', 'name' : 'the name'}
	//(32) : b' \x00\x00\x00\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x00'
	std::uint8_t data[] = { 0x1d, 0x0, 0x0, 0x0, 0x8, 0x69, 0x64, 0x0, 0x1, 0x2, 0x6e, 0x61, 0x6d, 0x65, 0x0, 0x9, 0x0, 0x0, 0x0, 0x74, 0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0, 0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto docLen = bobl::utility::read<std::uint32_t>(begin, end);
	BOOST_CHECK_EQUAL(docLen, sizeof(data));
	auto o = bobl::bson::flyweight::NameValue<bool, bobl::options::None>::decode(begin, end);
	BOOST_CHECK_EQUAL(o.name(), std::string{ "id" });
	BOOST_CHECK(o.value());
	auto any = bobl::bson::flyweight::NameValue<bobl::flyweight::lite::Any<decltype(begin)>, bobl::options::None>::decode(begin, end);
	BOOST_CHECK_EQUAL(any.name(), std::string{ "name" });
	BOOST_CHECK_EQUAL(*begin, 0);
	BOOST_CHECK_EQUAL(++begin, end);
}

BOOST_AUTO_TEST_CASE(RawAnyTest)
{
	//{'id': 'true', 'name' : 'the name'}
	//(32) : b' \x00\x00\x00\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x00'
	std::uint8_t data[] = { 0x1d, 0x0, 0x0, 0x0, 0x8, 0x69, 0x64, 0x0, 0x1, 0x2, 0x6e, 0x61, 0x6d, 0x65, 0x0, 0x9, 0x0, 0x0, 0x0, 0x74, 0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0, 0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	static_assert(!bobl::bson::flyweight::details::IsNameValue<bool>::value, "bobl::bson::flyweight::details::IsNameValue seems broken");
	static_assert(bobl::bson::flyweight::details::IsNameValue<bobl::bson::flyweight::NameValue<bool, bobl::options::None>>::value, "bobl::bson::flyweight::details::IsNameValue seems broken");
	auto docLen = bobl::utility::read<std::uint32_t>(begin, end);
	BOOST_CHECK_EQUAL(docLen, sizeof(data));
	{
		auto any = bobl::bson::flyweight::NameValue<bobl::flyweight::lite::Any<decltype(begin)>, bobl::options::None>::decode(begin, end);
		BOOST_CHECK_EQUAL(any.name(), std::string{ "id" });
		//auto o = bobl::bson::cast<bobl::bson::flyweight::NameValue<bool, bobl::options::None>>(any.value());
		auto o = bobl::bson::cast<bobl::flyweight::NameValue<bool>>(any.value());
		BOOST_CHECK_EQUAL(o.name(), std::string{ "id" });
		BOOST_CHECK(o.value());
		BOOST_CHECK(bobl::bson::cast<bool>(any.value()));
	}
	{
		auto any = bobl::bson::flyweight::NameValue<bobl::flyweight::lite::Any<decltype(begin)>, bobl::options::None>::decode(begin, end);
		BOOST_CHECK_EQUAL(any.name(), std::string{ "name" });
	}
	BOOST_CHECK_EQUAL(*begin, 0);
	BOOST_CHECK_EQUAL(++begin, end);
}

BOOST_AUTO_TEST_CASE(DocumentTest)
{
	//{'id': 'true', 'name' : 'the name'}
	//(32) : b' \x00\x00\x00\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x00'
	std::uint8_t data[] = { 0x1d, 0x0, 0x0, 0x0, 0x8, 0x69, 0x64, 0x0, 0x1, 0x2, 0x6e, 0x61, 0x6d, 0x65, 0x0, 0x9, 0x0, 0x0, 0x0, 0x74, 0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0, 0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	{
		auto res = bobl::bson::cast<std::tuple<bool>>(doc);
		BOOST_CHECK(std::get<0>(res));
	}
	{
		auto res = bobl::bson::cast<bool>(doc);
		BOOST_CHECK(res);
	}
	{
		auto res = bobl::bson::cast<bool, bobl::options::None>(doc);
		BOOST_CHECK(res);
	}
	BOOST_CHECK_THROW((bobl::bson::cast<bool, bobl::Options<bobl::options::ExacMatch>>(doc)), bobl::InvalidObject);
	{
		auto res = bobl::bson::cast<std::tuple<bool, bobl::bson::flyweight::NameValue<bobl::flyweight::lite::Any<uint8_t const*>, bobl::options::None>>>(doc);
		BOOST_CHECK(std::get<0>(res));
		BOOST_CHECK_EQUAL(std::get<1>(res).name(), std::string{ "name" });
	}
	{
		auto res = bobl::bson::cast<std::tuple<bool, bobl::bson::flyweight::NameValue<bobl::flyweight::lite::Any<uint8_t const*>, bobl::options::None>>, bobl::Options<bobl::options::ExacMatch>>(doc);
		BOOST_CHECK(std::get<0>(res));
		BOOST_CHECK_EQUAL(std::get<1>(res).name(), std::string{ "name" });
	}
	{
		auto res = bobl::bson::cast<std::tuple<bool, bobl::bson::flyweight::NameValue<bobl::flyweight::lite::Any<uint8_t const*>, bobl::options::None>>, bobl::Options<bobl::options::ExacMatch>>(doc);
		BOOST_CHECK(std::get<0>(res));
		BOOST_CHECK_EQUAL(std::get<1>(res).name(), std::string{ "name" });
	}
	BOOST_CHECK_THROW((bobl::bson::cast<std::tuple<bool>, bobl::Options<bobl::options::ExacMatch>>(doc)), bobl::InvalidObject);
}


BOOST_AUTO_TEST_CASE(LongIntTest)
{
	//{'int64': 9223372036854775807}
	//(20):b'\x14\x00\x00\x00\x12int64\x00\xff\xff\xff\xff\xff\xff\xff\x7f\x00'
	std::uint8_t data[] = {0x14, 0x0, 0x0, 0x0, 0x12, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x0};
	uint8_t const *begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_THROW((bobl::bson::cast<std::tuple<int>>(doc)), bobl::IncorrectObjectType);
	auto res = bobl::bson::cast<std::tuple<std::uint64_t>>(doc);
	BOOST_CHECK_EQUAL(std::get<0>(res), 9223372036854775807LL);
	BOOST_CHECK_EQUAL(bobl::bson::cast<std::uint64_t>(doc), 9223372036854775807LL);
	BOOST_CHECK_EQUAL(bobl::bson::cast<std::int64_t>(doc), 9223372036854775807LL);
	BOOST_CHECK_EQUAL(bobl::bson::cast<long long>(doc), 9223372036854775807LL);
	BOOST_CHECK_EQUAL((bobl::bson::cast<std::uint64_t, bobl::Options<bobl::options::RelaxedIntegers>>(doc)), 9223372036854775807LL);
	BOOST_CHECK_EQUAL((bobl::bson::cast<std::int64_t, bobl::Options<bobl::options::RelaxedIntegers>>(doc)), 9223372036854775807LL);
	BOOST_CHECK_EQUAL((bobl::bson::cast<long long, bobl::Options<bobl::options::RelaxedIntegers>>(doc)), 9223372036854775807LL);
	auto val = bobl::bson::cast<diversion::optional<std::uint64_t>, bobl::Options<bobl::options::RelaxedIntegers>>(doc);
	BOOST_CHECK_EQUAL(!val, false);
	BOOST_CHECK_EQUAL(val.get(), 9223372036854775807LL);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(RelaxedIntTest)
{
	//{'int': 1}
	//(14) : b'\x0e\x00\x00\x00\x10int\x00\x01\x00\x00\x00\x00'
	std::uint8_t data[] = { 0xe, 0x0, 0x0, 0x0, 0x10, 0x69, 0x6e, 0x74, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0 };
	uint8_t const *begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_THROW((bobl::bson::cast<std::tuple<std::uint64_t>>(doc)), bobl::IncorrectObjectType);
	BOOST_CHECK_THROW((bobl::bson::cast<std::uint64_t>(doc)), bobl::IncorrectObjectType);
	BOOST_CHECK_THROW((bobl::bson::cast<std::int64_t>(doc)), bobl::IncorrectObjectType);
	BOOST_CHECK_THROW((bobl::bson::cast<long long>(doc)), bobl::IncorrectObjectType);
	auto res = bobl::bson::cast<std::uint64_t, bobl::Options<bobl::options::RelaxedIntegers>>(doc);
	BOOST_CHECK_EQUAL(res, 1);
	BOOST_CHECK_EQUAL(begin, end);
}

template<typename T>
struct CheckValueVisitor : public boost::static_visitor<bool>
{
public:
	CheckValueVisitor(T expected) : expected_{ expected } {}
	bool operator()(T value) const { return expected_ == value; }
	template<typename U>
	bool operator()(U /*handler*/) const { return false; }
private:
	T expected_;
};

BOOST_AUTO_TEST_CASE(VariantTest)
{
	//{'int': 1}
	//(14) : b'\x0e\x00\x00\x00\x10int\x00\x01\x00\x00\x00\x00'
	std::uint8_t data[] = { 0xe, 0x0, 0x0, 0x0, 0x10, 0x69, 0x6e, 0x74, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0 };
	uint8_t const *begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	auto res = bobl::bson::cast<diversion::variant<bool, std::uint64_t, std::string, int>>(doc);
	BOOST_CHECK(diversion::visit(CheckValueVisitor<int>{1}, res));
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(EnumAdapterTest)
{
	//{'int': 1}
	//(14) : b'\x0e\x00\x00\x00\x10int\x00\x01\x00\x00\x00\x00'
	std::uint8_t data[] = { 0xe, 0x0, 0x0, 0x0, 0x10, 0x69, 0x6e, 0x74, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0 };
	uint8_t const *begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(bobl::bson::cast<TheEnum>(doc), 1);
	BOOST_CHECK(bobl::bson::cast<TheEnumClass>(doc) == TheEnumClass::One);
	BOOST_CHECK_EQUAL(!bobl::bson::cast<diversion::optional<TheEnum>>(doc), false);
	BOOST_CHECK_EQUAL(bobl::bson::cast<diversion::optional<TheEnum>>(doc).get(), 1);
	BOOST_CHECK_EQUAL(!bobl::bson::cast<diversion::optional<TheEnumClass>>(doc), false);
	BOOST_CHECK(bobl::bson::cast<diversion::optional<TheEnumClass>>(doc).get() == TheEnumClass::One);

	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(IntArrayTest)
{
	//////////BSON////////////
	//{'array': [0, 1, 2, 101]}
	//(45) :
	//b'-\x00\x00\x00\x04array\x00!\x00\x00\x00\x100\x00\x00\x00\x00\x00\x101\x00\x01\x00\x00\x00\x102\x00\x02\x00\x00\x00\x103\x00e\x00\x00\x00\x00\x00'
	std::uint8_t data[] = {0x2d, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61,
							0x79, 0x0,  0x21, 0x0,  0x0,  0x0,  0x10, 0x30, 0x0,
							0x0,  0x0,  0x0,  0x0,  0x10, 0x31, 0x0,  0x1,  0x0,
							0x0,  0x0,  0x10, 0x32, 0x0,  0x2,  0x0,  0x0,  0x0,
							0x10, 0x33, 0x0,  0x65, 0x0,  0x0,  0x0,  0x0,  0x0};


	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	auto res = bobl::bson::cast<std::vector<int>>(doc);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(res[0], 0);
	BOOST_CHECK_EQUAL(res[1], 1);
	BOOST_CHECK_EQUAL(res[2], 2);
	BOOST_CHECK_EQUAL(res[3], 101);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(IntArrayAsAnyArrayTest)
{
	//////////BSON////////////
	//{'array': [0, 1, 2, 101]}
	//(45) :
	//b'-\x00\x00\x00\x04array\x00!\x00\x00\x00\x100\x00\x00\x00\x00\x00\x101\x00\x01\x00\x00\x00\x102\x00\x02\x00\x00\x00\x103\x00e\x00\x00\x00\x00\x00'
	std::uint8_t data[] = {0x2d, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61,
							0x79, 0x0,  0x21, 0x0,  0x0,  0x0,  0x10, 0x30, 0x0,
							0x0,  0x0,  0x0,  0x0,  0x10, 0x31, 0x0,  0x1,  0x0,
							0x0,  0x0,  0x10, 0x32, 0x0,  0x2,  0x0,  0x0,  0x0,
							0x10, 0x33, 0x0,  0x65, 0x0,  0x0,  0x0,  0x0,  0x0};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	auto res = bobl::bson::cast<std::vector<bobl::flyweight::lite::Any<decltype(begin)>>>(doc);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(res[0]), 0);
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(res[1]), 1);
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(res[2]), 2);
	BOOST_CHECK_EQUAL(bobl::bson::cast<int>(res[3]), 101);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(FloatArrayTest)
{
	//////////BSON////////////
	//{'array': [0.1, 5555.77777, 2.1, 101.1234567]}
	//(61) :
	//b'=\x00\x00\x00\x04array\x001\x00\x00\x00\x010\x00\x9a\x99\x99\x99\x99\x99\xb9?\x011\x00\xcfI\xef\x1b\xc7\xb3\xb5@\x012\x00\xcd\xcc\xcc\xcc\xcc\xcc\x00@\x013\x007>\xee\xb6\xe6GY@\x00\x00'
	std::uint8_t data[] = {0x3d, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61,
							0x79, 0x0,  0x31, 0x0,  0x0,  0x0,  0x1,  0x30, 0x0,
							0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xb9, 0x3f, 0x1,
							0x31, 0x0,  0xcf, 0x49, 0xef, 0x1b, 0xc7, 0xb3, 0xb5,
							0x40, 0x1,  0x32, 0x0,  0xcd, 0xcc, 0xcc, 0xcc, 0xcc,
							0xcc, 0x0,  0x40, 0x1,  0x33, 0x0,  0x37, 0x3e, 0xee,
							0xb6, 0xe6, 0x47, 0x59, 0x40, 0x0,  0x0};


	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<std::vector<double>>(doc);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(res[0], 0.1);
	BOOST_CHECK_EQUAL(res[1], 5555.77777);
	BOOST_CHECK_EQUAL(res[2], 2.1);
	BOOST_CHECK_EQUAL(res[3], 101.1234567);
}


BOOST_AUTO_TEST_CASE(FloatArrayAsAnyTest)
{
	//////////BSON////////////
	//{'array': [0.1, 5555.77777, 2.1, 101.1234567]}
	//(61) :
	//b'=\x00\x00\x00\x04array\x001\x00\x00\x00\x010\x00\x9a\x99\x99\x99\x99\x99\xb9?\x011\x00\xcfI\xef\x1b\xc7\xb3\xb5@\x012\x00\xcd\xcc\xcc\xcc\xcc\xcc\x00@\x013\x007>\xee\xb6\xe6GY@\x00\x00'
	std::uint8_t data[] = {0x3d, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61,
							0x79, 0x0,  0x31, 0x0,  0x0,  0x0,  0x1,  0x30, 0x0,
							0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xb9, 0x3f, 0x1,
							0x31, 0x0,  0xcf, 0x49, 0xef, 0x1b, 0xc7, 0xb3, 0xb5,
							0x40, 0x1,  0x32, 0x0,  0xcd, 0xcc, 0xcc, 0xcc, 0xcc,
							0xcc, 0x0,  0x40, 0x1,  0x33, 0x0,  0x37, 0x3e, 0xee,
							0xb6, 0xe6, 0x47, 0x59, 0x40, 0x0,  0x0};


	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<std::vector<bobl::flyweight::lite::Any<decltype(begin)>>>(doc);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(bobl::bson::cast<double>(res[0]), 0.1);
//	BOOST_CHECK_EQUAL(bobl::bson::cast<float>(res[0]), 0.1);
	BOOST_CHECK_EQUAL(bobl::bson::cast<double>(res[1]), 5555.77777);
//	BOOST_CHECK_EQUAL(bobl::bson::cast<float>(res[1]), 5555.77777);
	BOOST_CHECK_EQUAL(bobl::bson::cast<double>(res[2]), 2.1);
//	BOOST_CHECK_EQUAL(bobl::bson::cast<float>(res[2]), 2.1);
	BOOST_CHECK_EQUAL(bobl::bson::cast<double>(res[3]), 101.1234567);
//	BOOST_CHECK_EQUAL(bobl::bson::cast<float>(res[3]), 101.1234567);
}

BOOST_AUTO_TEST_CASE(ArraysOfSimpleTest)
{
  //////////BSON////////////
  //{'array': [{'enabled': True, 'id': 10, 'name': 'first', 'theEnum': 1},
  //{'enabled': False, 'id': 20, 'name': 'second', 'theEnum': 2}, {'enabled':
  //True, 'id': 30, 'name': 'third', 'theEnum': 3}]} (183) :
  //b'\xb7\x00\x00\x00\x04array\x00\xab\x00\x00\x00\x030\x004\x00\x00\x00\x08enabled\x00\x01\x10id\x00\n\x00\x00\x00\x02name\x00\x06\x00\x00\x00first\x00\x10theEnum\x00\x01\x00\x00\x00\x00\x031\x005\x00\x00\x00\x08enabled\x00\x00\x10id\x00\x14\x00\x00\x00\x02name\x00\x07\x00\x00\x00second\x00\x10theEnum\x00\x02\x00\x00\x00\x00\x032\x004\x00\x00\x00\x08enabled\x00\x01\x10id\x00\x1e\x00\x00\x00\x02name\x00\x06\x00\x00\x00third\x00\x10theEnum\x00\x03\x00\x00\x00\x00\x00\x00'
  std::uint8_t data[] = {
						0xb7, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61, 0x79, 0x0,  0xab,
						0x0,  0x0,  0x0,  0x3,  0x30, 0x0,  0x34, 0x0,  0x0,  0x0,  0x8,  0x65,
						0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,
						0xa,  0x0,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x6,  0x0,
						0x0,  0x0,  0x66, 0x69, 0x72, 0x73, 0x74, 0x0,  0x10, 0x74, 0x68, 0x65,
						0x45, 0x6e, 0x75, 0x6d, 0x0,  0x1,  0x0,  0x0,  0x0,  0x0,  0x3,  0x31,
						0x0,  0x35, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
						0x64, 0x0,  0x0,  0x10, 0x69, 0x64, 0x0,  0x14, 0x0,  0x0,  0x0,  0x2,
						0x6e, 0x61, 0x6d, 0x65, 0x0,  0x7,  0x0,  0x0,  0x0,  0x73, 0x65, 0x63,
						0x6f, 0x6e, 0x64, 0x0,  0x10, 0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 0x6d,
						0x0,  0x2,  0x0,  0x0,  0x0,  0x0,  0x3,  0x32, 0x0,  0x34, 0x0,  0x0,
						0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0x0,  0x1,  0x10,
						0x69, 0x64, 0x0,  0x1e, 0x0,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65,
						0x0,  0x6,  0x0,  0x0,  0x0,  0x74, 0x68, 0x69, 0x72, 0x64, 0x0,  0x10,
						0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x3,  0x0,  0x0,  0x0,
						0x0,  0x0,  0x0};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<std::vector<Simple>>(doc);

	BOOST_CHECK_EQUAL(res.size(), 3);
	BOOST_CHECK(res[0].enabled);
	BOOST_CHECK_EQUAL(res[0].id, 10);
	BOOST_CHECK_EQUAL(res[0].name, std::string{"first"});
	BOOST_CHECK_EQUAL(int(res[0].theEnum), 1);

	BOOST_CHECK(!res[1].enabled);
	BOOST_CHECK_EQUAL(res[1].id, 20);
	BOOST_CHECK_EQUAL(res[1].name, std::string{"second"});
	BOOST_CHECK_EQUAL(int(res[1].theEnum), 2);

	BOOST_CHECK(res[2].enabled);
	BOOST_CHECK_EQUAL(res[2].id, 30);
	BOOST_CHECK_EQUAL(res[2].name, std::string{"third"});
	BOOST_CHECK_EQUAL(int(res[2].theEnum), 3);
}

BOOST_AUTO_TEST_CASE(ArraysOfSimpleAsAnyTest)
{
	//////////BSON////////////
	//{'array': [{'enabled': True, 'id': 10, 'name': 'first', 'theEnum': 1},
	//{'enabled': False, 'id': 20, 'name': 'second', 'theEnum': 2}, {'enabled':
	//True, 'id': 30, 'name': 'third', 'theEnum': 3}]} (183) :
	//b'\xb7\x00\x00\x00\x04array\x00\xab\x00\x00\x00\x030\x004\x00\x00\x00\x08enabled\x00\x01\x10id\x00\n\x00\x00\x00\x02name\x00\x06\x00\x00\x00first\x00\x10theEnum\x00\x01\x00\x00\x00\x00\x031\x005\x00\x00\x00\x08enabled\x00\x00\x10id\x00\x14\x00\x00\x00\x02name\x00\x07\x00\x00\x00second\x00\x10theEnum\x00\x02\x00\x00\x00\x00\x032\x004\x00\x00\x00\x08enabled\x00\x01\x10id\x00\x1e\x00\x00\x00\x02name\x00\x06\x00\x00\x00third\x00\x10theEnum\x00\x03\x00\x00\x00\x00\x00\x00'
	std::uint8_t data[] = {
						0xb7, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61, 0x79, 0x0,  0xab,
						0x0,  0x0,  0x0,  0x3,  0x30, 0x0,  0x34, 0x0,  0x0,  0x0,  0x8,  0x65,
						0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,
						0xa,  0x0,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x6,  0x0,
						0x0,  0x0,  0x66, 0x69, 0x72, 0x73, 0x74, 0x0,  0x10, 0x74, 0x68, 0x65,
						0x45, 0x6e, 0x75, 0x6d, 0x0,  0x1,  0x0,  0x0,  0x0,  0x0,  0x3,  0x31,
						0x0,  0x35, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
						0x64, 0x0,  0x0,  0x10, 0x69, 0x64, 0x0,  0x14, 0x0,  0x0,  0x0,  0x2,
						0x6e, 0x61, 0x6d, 0x65, 0x0,  0x7,  0x0,  0x0,  0x0,  0x73, 0x65, 0x63,
						0x6f, 0x6e, 0x64, 0x0,  0x10, 0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 0x6d,
						0x0,  0x2,  0x0,  0x0,  0x0,  0x0,  0x3,  0x32, 0x0,  0x34, 0x0,  0x0,
						0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0x0,  0x1,  0x10,
						0x69, 0x64, 0x0,  0x1e, 0x0,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65,
						0x0,  0x6,  0x0,  0x0,  0x0,  0x74, 0x68, 0x69, 0x72, 0x64, 0x0,  0x10,
						0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x3,  0x0,  0x0,  0x0,
						0x0,  0x0,  0x0};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<std::vector<bobl::flyweight::lite::Any<decltype(begin)>>>(doc);
	BOOST_CHECK_EQUAL(res.size(), 3);
	{
		auto simple = bobl::bson::cast<Simple>(res[0]);
		BOOST_CHECK(simple.enabled);
		BOOST_CHECK_EQUAL(simple.id, 10);
		BOOST_CHECK_EQUAL(simple.name, std::string{ "first" });
		BOOST_CHECK_EQUAL(int(simple.theEnum), 1);
	}
	{
		auto simple = bobl::bson::cast<Simple>(res[1]);
		BOOST_CHECK(!simple.enabled);
		BOOST_CHECK_EQUAL(simple.id, 20);
		BOOST_CHECK_EQUAL(simple.name, std::string{ "second" });
		BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
	}
	{
		auto simple = bobl::bson::cast<Simple>(res[2]);
		BOOST_CHECK(simple.enabled);
		BOOST_CHECK_EQUAL(simple.id, 30);
		BOOST_CHECK_EQUAL(simple.name, std::string{ "third" });
		BOOST_CHECK_EQUAL(int(simple.theEnum), 3);
	}
}



BOOST_AUTO_TEST_CASE(DictionaryOfRelaxedIntegersEncodedInSameOrderTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
    std::uint8_t data[] = {
						0x69, 0x0,  0x0,  0x0,  0x10, 0x74, 0x69, 0x6e, 0x79, 0x0,  0x3,
						0x0,  0x0,  0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
						0x80, 0x0,  0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x5f, 0x0,  0xff,
						0x7f, 0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0xff,
						0xff, 0xff, 0xf,  0x10, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e,
						0x79, 0x0,  0xfc, 0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f,
						0x73, 0x68, 0x6f, 0x72, 0x74, 0x0,  0x87, 0xff, 0xff, 0xff, 0x10,
						0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x0,  0x3,  0x80, 0xff,
						0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34,
						0x0,  0x2,  0x0,  0x0,  0xf0, 0x0};


	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<OrderedIntDictionary,bobl::Options<bobl::options::RelaxedIntegers>>(doc);
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
	BOOST_CHECK_EQUAL(res.int_, 32767);
	BOOST_CHECK_EQUAL(res.int64, 268435455);
	BOOST_CHECK_EQUAL(res.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.neg_short, -121);
	BOOST_CHECK_EQUAL(res.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
}

BOOST_AUTO_TEST_CASE(DictionaryOfRelaxedIntegers)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
    std::uint8_t data[] = {
			0x69, 0x0,  0x0,  0x0,  0x10, 0x74, 0x69, 0x6e, 0x79, 0x0,  0x3,
			0x0,  0x0,  0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
			0x80, 0x0,  0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x5f, 0x0,  0xff,
			0x7f, 0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0xff,
			0xff, 0xff, 0xf,  0x10, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e,
			0x79, 0x0,  0xfc, 0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f,
			0x73, 0x68, 0x6f, 0x72, 0x74, 0x0,  0x87, 0xff, 0xff, 0xff, 0x10,
			0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x0,  0x3,  0x80, 0xff,
			0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34,
			0x0,  0x2,  0x0,  0x0,  0xf0, 0x0};


    uint8_t const *begin = data;
    uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<OrderedIntDictionary,bobl::Options<bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers>>(doc);
    BOOST_CHECK_EQUAL(res.tiny, 3);
    BOOST_CHECK_EQUAL(res.short_, 128);
    BOOST_CHECK_EQUAL(res.int_, 32767);
    BOOST_CHECK_EQUAL(res.int64, 268435455);
    BOOST_CHECK_EQUAL(res.neg_tiny, -4);
    BOOST_CHECK_EQUAL(res.neg_short, -121);
    BOOST_CHECK_EQUAL(res.neg_int, -32765);
    BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
}

BOOST_AUTO_TEST_CASE(DictionaryOfRelaxedIntegersEncodedInDifferentOrderTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
    std::uint8_t data[] = {
        0x69, 0x0,  0x0,  0x0,  0x10, 0x74, 0x69, 0x6e, 0x79, 0x0,  0x3,
        0x0,  0x0,  0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
        0x80, 0x0,  0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x5f, 0x0,  0xff,
        0x7f, 0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0xff,
        0xff, 0xff, 0xf,  0x10, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e,
        0x79, 0x0,  0xfc, 0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f,
        0x73, 0x68, 0x6f, 0x72, 0x74, 0x0,  0x87, 0xff, 0xff, 0xff, 0x10,
        0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x0,  0x3,  0x80, 0xff,
        0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34,
        0x0,  0x2,  0x0,  0x0,  0xf0, 0x0};


    uint8_t const *begin = data;
    uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<IntDictionary,bobl::Options<bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers>>(doc);
    BOOST_CHECK_EQUAL(res.tiny, 3);
    BOOST_CHECK_EQUAL(res.short_, 128);
    BOOST_CHECK_EQUAL(res.int_, 32767);
    BOOST_CHECK_EQUAL(res.int64, 268435455);
    BOOST_CHECK_EQUAL(res.neg_tiny, -4);
    BOOST_CHECK_EQUAL(res.neg_short, -121);
    BOOST_CHECK_EQUAL(res.neg_int, -32765);
    BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
}


BOOST_AUTO_TEST_CASE(OrderedFlotDictionaryTest, *boost::unit_test::tolerance(0.00001))
{
	//////////BSON////////////
	//{'nul': 0.0, 'short_': 3.333, 'short_max': 65504.0, 'f32': 32767.0, 'f64': 2147483650.99999, 'neg_short': -3.555}
	//(98) : b'b\x00\x00\x00\x01nul\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01short_\x00D\x8bl\xe7\xfb\xa9\n@\x01short_max\x00\x00\x00\x00\x00\x00\xfc\xef@\x01f32\x00\x00\x00\x00\x00\xc0\xff\xdf@\x01f64\x00\xeb\xff_\x00\x00\x00\xe0A\x01neg_short\x00q=\n\xd7\xa3p\x0c\xc0\x00'
    std::uint8_t data[] = {
					0x62, 0x0,  0x0,  0x0,  0x1,  0x6e, 0x75, 0x6c, 0x0,  0x0,  0x0,
					0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x73, 0x68, 0x6f, 0x72,
					0x74, 0x5f, 0x0,  0x44, 0x8b, 0x6c, 0xe7, 0xfb, 0xa9, 0xa,  0x40,
					0x1,  0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x6d, 0x61, 0x78, 0x0,
					0x0,  0x0,  0x0,  0x0,  0x0,  0xfc, 0xef, 0x40, 0x1,  0x66, 0x33,
					0x32, 0x0,  0x0,  0x0,  0x0,  0x0,  0xc0, 0xff, 0xdf, 0x40, 0x1,
					0x66, 0x36, 0x34, 0x0,  0xeb, 0xff, 0x5f, 0x0,  0x0,  0x0,  0xe0,
					0x41, 0x1,  0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74,
					0x0,  0x71, 0x3d, 0xa,  0xd7, 0xa3, 0x70, 0xc,  0xc0, 0x0};

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
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

BOOST_AUTO_TEST_CASE(RawSimpleTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
    std::uint8_t data[] = {
					0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
					0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
					0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
					0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
					0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0};
    uint8_t const *begin = data;
    uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = bobl::bson::cast<std::tuple<bool, int, std::string, TheEnumClass>>(doc);
	BOOST_CHECK(std::get<0>(simple));
	BOOST_CHECK_EQUAL(std::get<1>(simple), 100);
	BOOST_CHECK_EQUAL(std::get<2>(simple), std::string{ "the name" });
	BOOST_CHECK(std::get<3>(simple) == TheEnumClass::Two);
}

BOOST_AUTO_TEST_CASE(SimpleTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
    std::uint8_t data[] = {
					0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
					0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
					0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
					0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
					0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0};
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto simple = bobl::bson::cast<Simple>(doc);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
}

BOOST_AUTO_TEST_CASE(SimpleAsTupleTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
	std::uint8_t data[] = {
		0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
		0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
		0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
		0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
		0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::bson::decode<std::tuple<bool, int, std::string, TheEnumClass>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(std::get<0>(res));
	BOOST_CHECK_EQUAL(std::get<1>(res), 100);
	BOOST_CHECK_EQUAL(std::get<2>(res), std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(std::get<3>(res)), 2);
}

BOOST_AUTO_TEST_CASE(SimpleAsImplicitTupleTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
    std::uint8_t data[] = {
					0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
					0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
					0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
					0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
					0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0};
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::bson::decode<bool, int, std::string, TheEnumClass>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(std::get<0>(res));
	BOOST_CHECK_EQUAL(std::get<1>(res), 100);
	BOOST_CHECK_EQUAL(std::get<2>(res), std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(std::get<3>(res)), 2);
}

BOOST_AUTO_TEST_CASE(SimpleOptioonalTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
    std::uint8_t data[] = {
					0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
					0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
					0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
					0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
					0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0};
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);

	auto simple = bobl::bson::cast<SimpleOptional>(doc);
	BOOST_CHECK_EQUAL(!simple.enabled, false);
	BOOST_CHECK(simple.enabled.get());
	BOOST_CHECK_EQUAL(!simple.id, false);
	BOOST_CHECK_EQUAL(simple.id.get(), 100);
	BOOST_CHECK_EQUAL(!simple.name, false);
	BOOST_CHECK_EQUAL(simple.name.get(), std::string{ "the name" });
	BOOST_CHECK_EQUAL(!simple.theEnum, false);
	BOOST_CHECK(simple.theEnum.get() == TheEnumClass::Two);
	BOOST_CHECK(!simple.dummy1);
	BOOST_CHECK(!simple.dummy2);
	BOOST_CHECK(!simple.dummy3);


	auto res = bobl::bson::cast<diversion::optional<std::string>, diversion::optional<bool>, diversion::optional<std::vector<int>>
														, diversion::optional<std::vector<Simple>>>(doc);
	BOOST_CHECK(!std::get<0>(res));
	BOOST_CHECK_EQUAL(!std::get<1>(res), false);
	BOOST_CHECK(!std::get<2>(res));
	BOOST_CHECK(!std::get<3>(res));
}



BOOST_AUTO_TEST_CASE(SimpleXTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
    std::uint8_t data[] = {
					0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
					0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
					0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
					0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
					0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0};
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	auto simple = bobl::bson::cast<SimpleVariant>(doc);
	BOOST_CHECK(boost::get<bool>(simple.enabled));
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(boost::get<std::string>(simple.name), std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(boost::get<TheEnumClass>(simple.theEnum)), 2);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(NestedOrderedDictTest)
{
	//{'name': 'name of something', 'ints': {'tiny': 3, 'short_': 128, 'int_':
	//32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int':
	//-32765, 'neg_int64': -268435454}, 'level1': {'name': 'level1', 'short_':
	//123, 'level2': {'name': 'level2', 'short_': -255, 'level3': {'name':
	//'level3', 'short_': -1}}}} (270) :
	//b'\x0e\x01\x00\x00\x02name\x00\x12\x00\x00\x00name of
	//something\x00\x03ints\x00i\x00\x00\x00\x10tiny\x00\x03\x00\x00\x00\x10short_\x00\x80\x00\x00\x00\x10int_\x00\xff\x7f\x00\x00\x10int64\x00\xff\xff\xff\x0f\x10neg_tiny\x00\xfc\xff\xff\xff\x10neg_short\x00\x87\xff\xff\xff\x10neg_int\x00\x03\x80\xff\xff\x10neg_int64\x00\x02\x00\x00\xf0\x00\x03level1\x00v\x00\x00\x00\x02name\x00\x07\x00\x00\x00level1\x00\x10short_\x00{\x00\x00\x00\x03level2\x00L\x00\x00\x00\x02name\x00\x07\x00\x00\x00level2\x00\x10short_\x00\x01\xff\xff\xff\x03level3\x00"\x00\x00\x00\x02name\x00\x07\x00\x00\x00level3\x00\x10short_\x00\xff\xff\xff\xff\x00\x00\x00\x00'
	std::uint8_t data[] = {
			0xe,  0x1,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x12, 0x0,
			0x0,  0x0,  0x6e, 0x61, 0x6d, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x73, 0x6f,
			0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x0,  0x3,  0x69, 0x6e, 0x74,
			0x73, 0x0,  0x69, 0x0,  0x0,  0x0,  0x10, 0x74, 0x69, 0x6e, 0x79, 0x0,
			0x3,  0x0,  0x0,  0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
			0x80, 0x0,  0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x5f, 0x0,  0xff, 0x7f,
			0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0xff, 0xff, 0xff,
			0xf,  0x10, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x0,  0xfc,
			0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72,
			0x74, 0x0,  0x87, 0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x69,
			0x6e, 0x74, 0x0,  0x3,  0x80, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f,
			0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0x2,  0x0,  0x0,  0xf0, 0x0,  0x3,
			0x6c, 0x65, 0x76, 0x65, 0x6c, 0x31, 0x0,  0x76, 0x0,  0x0,  0x0,  0x2,
			0x6e, 0x61, 0x6d, 0x65, 0x0,  0x7,  0x0,  0x0,  0x0,  0x6c, 0x65, 0x76,
			0x65, 0x6c, 0x31, 0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
			0x7b, 0x0,  0x0,  0x0,  0x3,  0x6c, 0x65, 0x76, 0x65, 0x6c, 0x32, 0x0,
			0x4c, 0x0,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x7,  0x0,
			0x0,  0x0,  0x6c, 0x65, 0x76, 0x65, 0x6c, 0x32, 0x0,  0x10, 0x73, 0x68,
			0x6f, 0x72, 0x74, 0x5f, 0x0,  0x1,  0xff, 0xff, 0xff, 0x3,  0x6c, 0x65,
			0x76, 0x65, 0x6c, 0x33, 0x0,  0x22, 0x0,  0x0,  0x0,  0x2,  0x6e, 0x61,
			0x6d, 0x65, 0x0,  0x7,  0x0,  0x0,  0x0,  0x6c, 0x65, 0x76, 0x65, 0x6c,
			0x33, 0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,  0xff, 0xff,
			0xff, 0xff, 0x0,  0x0,  0x0,  0x0};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<
		Nested</*IntDictionary*/>, bobl::Options</*bobl::options::StructAsDictionary,*/ bobl::options::RelaxedIntegers>>(
		doc);
	BOOST_CHECK_EQUAL(res.name, std::string{"name of something"});
	BOOST_CHECK_EQUAL(res.ints.tiny, 3);
	BOOST_CHECK_EQUAL(res.ints.short_, 128);
	BOOST_CHECK_EQUAL(res.ints.int_, 32767);
	BOOST_CHECK_EQUAL(res.ints.int64, 268435455);
	BOOST_CHECK_EQUAL(res.ints.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.ints.neg_short, -121);
	BOOST_CHECK_EQUAL(res.ints.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.ints.neg_int64, -268435454);
	BOOST_CHECK_EQUAL(res.level1.name, std::string{"level1"});
	BOOST_CHECK_EQUAL(res.level1.short_, 123);
	BOOST_CHECK_EQUAL(res.level1.level2.name, std::string{"level2"});
	BOOST_CHECK_EQUAL(res.level1.level2.short_, -255);
	BOOST_CHECK_EQUAL(res.level1.level2.level3.name, std::string{"level3"});
	BOOST_CHECK_EQUAL(res.level1.level2.level3.short_, -1);
}


BOOST_AUTO_TEST_CASE(NestedUnorderedDictTest)
{
	//{'name': 'name of something', 'ints': {'tiny': 3, 'short_': 128, 'int_':
	//32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int':
	//-32765, 'neg_int64': -268435454}, 'level1': {'name': 'level1', 'short_':
	//123, 'level2': {'name': 'level2', 'short_': -255, 'level3': {'name':
	//'level3', 'short_': -1}}}} (270) :
	//b'\x0e\x01\x00\x00\x02name\x00\x12\x00\x00\x00name of
	//something\x00\x03ints\x00i\x00\x00\x00\x10tiny\x00\x03\x00\x00\x00\x10short_\x00\x80\x00\x00\x00\x10int_\x00\xff\x7f\x00\x00\x10int64\x00\xff\xff\xff\x0f\x10neg_tiny\x00\xfc\xff\xff\xff\x10neg_short\x00\x87\xff\xff\xff\x10neg_int\x00\x03\x80\xff\xff\x10neg_int64\x00\x02\x00\x00\xf0\x00\x03level1\x00v\x00\x00\x00\x02name\x00\x07\x00\x00\x00level1\x00\x10short_\x00{\x00\x00\x00\x03level2\x00L\x00\x00\x00\x02name\x00\x07\x00\x00\x00level2\x00\x10short_\x00\x01\xff\xff\xff\x03level3\x00"\x00\x00\x00\x02name\x00\x07\x00\x00\x00level3\x00\x10short_\x00\xff\xff\xff\xff\x00\x00\x00\x00'
	std::uint8_t data[] = {
				0xe,  0x1,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x12, 0x0,
				0x0,  0x0,  0x6e, 0x61, 0x6d, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x73, 0x6f,
				0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x0,  0x3,  0x69, 0x6e, 0x74,
				0x73, 0x0,  0x69, 0x0,  0x0,  0x0,  0x10, 0x74, 0x69, 0x6e, 0x79, 0x0,
				0x3,  0x0,  0x0,  0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
				0x80, 0x0,  0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x5f, 0x0,  0xff, 0x7f,
				0x0,  0x0,  0x10, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0xff, 0xff, 0xff,
				0xf,  0x10, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x0,  0xfc,
				0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72,
				0x74, 0x0,  0x87, 0xff, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f, 0x69,
				0x6e, 0x74, 0x0,  0x3,  0x80, 0xff, 0xff, 0x10, 0x6e, 0x65, 0x67, 0x5f,
				0x69, 0x6e, 0x74, 0x36, 0x34, 0x0,  0x2,  0x0,  0x0,  0xf0, 0x0,  0x3,
				0x6c, 0x65, 0x76, 0x65, 0x6c, 0x31, 0x0,  0x76, 0x0,  0x0,  0x0,  0x2,
				0x6e, 0x61, 0x6d, 0x65, 0x0,  0x7,  0x0,  0x0,  0x0,  0x6c, 0x65, 0x76,
				0x65, 0x6c, 0x31, 0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,
				0x7b, 0x0,  0x0,  0x0,  0x3,  0x6c, 0x65, 0x76, 0x65, 0x6c, 0x32, 0x0,
				0x4c, 0x0,  0x0,  0x0,  0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x7,  0x0,
				0x0,  0x0,  0x6c, 0x65, 0x76, 0x65, 0x6c, 0x32, 0x0,  0x10, 0x73, 0x68,
				0x6f, 0x72, 0x74, 0x5f, 0x0,  0x1,  0xff, 0xff, 0xff, 0x3,  0x6c, 0x65,
				0x76, 0x65, 0x6c, 0x33, 0x0,  0x22, 0x0,  0x0,  0x0,  0x2,  0x6e, 0x61,
				0x6d, 0x65, 0x0,  0x7,  0x0,  0x0,  0x0,  0x6c, 0x65, 0x76, 0x65, 0x6c,
				0x33, 0x0,  0x10, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x0,  0xff, 0xff,
				0xff, 0xff, 0x0,  0x0,  0x0,  0x0};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	auto res = bobl::bson::cast<
		Nested<IntDictionary>, bobl::Options<bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers>>(
		doc);
	BOOST_CHECK_EQUAL(res.name, std::string{"name of something"});
	BOOST_CHECK_EQUAL(res.ints.tiny, 3);
	BOOST_CHECK_EQUAL(res.ints.short_, 128);
	BOOST_CHECK_EQUAL(res.ints.int_, 32767);
	BOOST_CHECK_EQUAL(res.ints.int64, 268435455);
	BOOST_CHECK_EQUAL(res.ints.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.ints.neg_short, -121);
	BOOST_CHECK_EQUAL(res.ints.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.ints.neg_int64, -268435454);
	BOOST_CHECK_EQUAL(res.level1.name, std::string{"level1"});
	BOOST_CHECK_EQUAL(res.level1.short_, 123);
	BOOST_CHECK_EQUAL(res.level1.level2.name, std::string{"level2"});
	BOOST_CHECK_EQUAL(res.level1.level2.short_, -255);
	BOOST_CHECK_EQUAL(res.level1.level2.level3.name, std::string{"level3"});
	BOOST_CHECK_EQUAL(res.level1.level2.level3.short_, -1);
}


BOOST_AUTO_TEST_CASE(BinaryTest)
{
	//(20):b'\x14\x00\x00\x00\x05bin\x00\x05\x00\x00\x00\x00bytes\x00'
	std::uint8_t data[] = { 0x14, 0x0, 0x0, 0x0, 0x5, 0x62, 0x69, 0x6e, 0x0, 0x5, 0x0, 0x0, 0x0, 0x0, 0x62, 0x79, 0x74, 0x65, 0x73, 0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::bson::decode<std::vector<uint8_t>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL(res.size(), 5);
	BOOST_CHECK_EQUAL(res[0], 0x62);
	BOOST_CHECK_EQUAL(res[1], 0x79);
	BOOST_CHECK_EQUAL(res[2], 0x74);
	BOOST_CHECK_EQUAL(res[3], 0x65);
	BOOST_CHECK_EQUAL(res[4], 0x73);
}

BOOST_AUTO_TEST_CASE(BinaryCharTest)
{
	//(20):b'\x14\x00\x00\x00\x05bin\x00\x05\x00\x00\x00\x00bytes\x00'
	std::uint8_t data[] = { 0x14, 0x0, 0x0, 0x0, 0x5, 0x62, 0x69, 0x6e, 0x0, 0x5, 0x0, 0x0, 0x0, 0x0, 0x62, 0x79, 0x74, 0x65, 0x73, 0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK_THROW((bobl::bson::decode<std::vector<char>>(begin, end)), bobl::IncorrectObjectType);

	/*uint8_t const**/ begin = data;
	auto expected = std::string{ "bytes" };
	auto res = bobl::bson::decode<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(expected), std::end(expected));
}

BOOST_AUTO_TEST_CASE(BinaryAsAnyTest)
{
	//(20):b'\x14\x00\x00\x00\x05bin\x00\x05\x00\x00\x00\x00bytes\x00'
	std::uint8_t data[] = { 0x14, 0x0, 0x0, 0x0, 0x5, 0x62, 0x69, 0x6e, 0x0, 0x5, 0x0, 0x0, 0x0, 0x0, 0x62, 0x79, 0x74, 0x65, 0x73, 0x0 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK_THROW((bobl::bson::decode<std::vector<char>>(begin, end)), bobl::IncorrectObjectType);

	/*uint8_t const**/ begin = data;
	auto expected = std::string{ "bytes" };
	auto any = bobl::bson::decode<bobl::flyweight::lite::Any<decltype(begin)>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_THROW((bobl::bson::cast<std::vector<char>>(any)), bobl::IncorrectObjectType);
	auto res = bobl::bson::cast<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(any);;
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(expected), std::end(expected));
}

BOOST_AUTO_TEST_CASE(UUIDVectorTest)
{
	//////////BSON////////////
	//(66):b'B\x00\x00\x00\x04vector\x005\x00\x00\x00\x050\x00\x10\x00\x00\x00\x03N\x980\x10\xfadK\xaa\xab\xed\xdd\x82\xfdi\x1d\x18\x051\x00\x10\x00\x00\x00\x03\x97\xb9\xb4\x88;ELP\x81Dq\x9a\x84\x14!l\x00\x00'
	std::uint8_t data[] = {
				0x42, 0x0,  0x0,  0x0,  0x4,  0x76, 0x65, 0x63, 0x74, 0x6f, 0x72,
				0x0,  0x35, 0x0,  0x0,  0x0,  0x5,  0x30, 0x0,  0x10, 0x0,  0x0,
				0x0,  0x3,  0x4e, 0x98, 0x30, 0x10, 0xfa, 0x64, 0x4b, 0xaa, 0xab,
				0xed, 0xdd, 0x82, 0xfd, 0x69, 0x1d, 0x18, 0x5,  0x31, 0x0,  0x10,
				0x0,  0x0,  0x0,  0x3,  0x97, 0xb9, 0xb4, 0x88, 0x3b, 0x45, 0x4c,
				0x50, 0x81, 0x44, 0x71, 0x9a, 0x84, 0x14, 0x21, 0x6c, 0x0,  0x0};

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK_THROW((bobl::bson::decode<Vector<int>>(begin, end)), bobl::IncorrectObjectType);

	boost::uuids::string_generator gen;
	auto expected = std::vector<boost::uuids::uuid>{ gen("4E983010-FA64-4BAA-ABED-DD82FD691D18"), gen("97B9B488-3B45-4C50-8144-719A8414216C") };
	/*uint8_t const**/ begin = data;
	auto res = bobl::bson::decode<Vector<boost::uuids::uuid>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.vector), std::end(res.vector), std::begin(expected), std::end(expected));
}

BOOST_AUTO_TEST_CASE(UUIDVectorTest2)
{
	//(66):b'B\x00\x00\x00\x04vector\x005\x00\x00\x00\x050\x00\x10\x00\x00\x00\x03\xea\xa8\x99v\xcf\xccC\xde\xa2;\x10\xc6{\x10\n\xe8\x051\x00\x10\x00\x00\x00\x03\x9d\x89H$7/N\xf7\xa1xy7\xf7\x05G\x83\x00\x00'
	std::uint8_t data[] = {
					0x42, 0x0,  0x0,  0x0,  0x4,  0x76, 0x65, 0x63, 0x74, 0x6f, 0x72,
					0x0,  0x35, 0x0,  0x0,  0x0,  0x5,  0x30, 0x0,  0x10, 0x0,  0x0,
					0x0,  0x3,  0xea, 0xa8, 0x99, 0x76, 0xcf, 0xcc, 0x43, 0xde, 0xa2,
					0x3b, 0x10, 0xc6, 0x7b, 0x10, 0xa,  0xe8, 0x5,  0x31, 0x0,  0x10,
					0x0,  0x0,  0x0,  0x3,  0x9d, 0x89, 0x48, 0x24, 0x37, 0x2f, 0x4e,
					0xf7, 0xa1, 0x78, 0x79, 0x37, 0xf7, 0x5,  0x47, 0x83, 0x0,  0x0};

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK_THROW((bobl::bson::decode<Vector<int>>(begin, end)), bobl::IncorrectObjectType);

	boost::uuids::string_generator gen;
	auto expected = std::vector<boost::uuids::uuid>{ gen("EAA89976-CFCC-43DE-A23B-10C67B100AE8"), gen("9D894824-372F-4EF7-A178-7937F7054783") };
	/*uint8_t const**/ begin = data;
	auto res = bobl::bson::decode<Vector<boost::uuids::uuid>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.vector), std::end(res.vector), std::begin(expected), std::end(expected));
}

BOOST_AUTO_TEST_SUITE_END()
