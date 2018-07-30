#include <boost/test/unit_test.hpp>
#include "tests.hpp"
#include "test_hlp.hpp"
#include "bobl/bson/iterator.hpp"
#include "bobl/bson/cast.hpp"
#include "bobl/bson/decode.hpp"
#include "bobl/bson/encode.hpp"
#include "bobl/bobl.hpp"
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <sstream>
#include <cstdint>


BOOST_AUTO_TEST_SUITE(BOBL_BSON_Iterators_TestSuite)

std::string cpp_type(bobl::Type type)
{
	auto res = std::string{ "bobl::flyweight::Any" };
	switch (type)
	{
		case bobl::Type::Bool:
			res = "std::string";
			break; 
		case bobl::Type::Tiny:
		case bobl::Type::Int8:
			res = "std::int8_t";
			break;
		case bobl::Type::Int16:
			res = "std::int16_t";
			break;
		case bobl::Type::Int32:
			res = "std::int32_t";
			break;
		case bobl::Type::Int64:
			res = "std::int64_t";
			break;
		case bobl::Type::UnsignedInt8:
			res = "std::uint8_t";
			break;
		case bobl::Type::UnsignedInt16:
			res = "std::uint16_t";
			break;
		case bobl::Type::UnsignedInt32:
			res = "std::uint32_t";
			break;
		case bobl::Type::UnsignedInt64:
			res = "std::uint64_t";
			break;
		case bobl::Type::Float8:
		case bobl::Type::Float16:
		case bobl::Type::Float32:
			res = "float";
			break;
		case bobl::Type::Float64:
			res = "double";
			break;
		case bobl::Type::String:
			res = "std::string";
			break;
		case bobl::Type::Binary:
			res = "std::vector<std::uint8_t>";
			break; 
		case bobl::Type::Timepoint:
			res = "std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>";
			break; 
		case bobl::Type::UUID:
			res = "boost::uuids::uuid";
			break;
		default:
			throw std::runtime_error(str(boost::format("unsupported type %1% (%2$#x)") % to_string(type) % int(type)));
			break;
	}
	return res;
}

void dump(bobl::Type type, bobl::flyweight::Any const& any)
{
	switch (type)
	{
		case bobl::Type::Bool:
			bobl::bson::cast<bool>(any);
			break; 
		case bobl::Type::Tiny:
		case bobl::Type::Int8:
			bobl::bson::cast<std::int8_t>(any);
			break;
		case bobl::Type::Int16:
			bobl::bson::cast<std::int16_t>(any);
			break;
		case bobl::Type::Int32:
			bobl::bson::cast<std::int32_t>(any);
			break;
		case bobl::Type::Int64:
			bobl::bson::cast<std::int32_t>(any);
			break;
		case bobl::Type::UnsignedInt8:
			bobl::bson::cast<std::uint8_t>(any);
			break;
		case bobl::Type::UnsignedInt16:
			bobl::bson::cast<std::uint16_t>(any);
			break;
		case bobl::Type::UnsignedInt32:
			bobl::bson::cast<std::uint32_t>(any);
			break;
		case bobl::Type::UnsignedInt64:
			bobl::bson::cast<std::uint64_t>(any);
			break;
		case bobl::Type::Float8:
		case bobl::Type::Float16:
		case bobl::Type::Float32:
			bobl::bson::cast<float>(any);
			break;
		case bobl::Type::Float64:
			bobl::bson::cast<double>(any);
			break;
		case bobl::Type::String:
			bobl::bson::cast<std::string>(any);
			break;
		case bobl::Type::Binary:
			bobl::bson::cast<std::vector<std::uint8_t>>(any);
			break; 
		case bobl::Type::Timepoint:
			bobl::bson::cast<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>>(any);
			break;
		case bobl::Type::UUID:
			bobl::bson::cast<boost::uuids::uuid>(any);
			break;
		default:
			throw std::runtime_error(str(boost::format("unsupported type %1% (%2$#x)") % to_string(type) % int(type)));
			break;
	}
}

std::ostream& dump(std::ostream& out, bobl::flyweight::Any const& any);

template<typename Iterator>
std::ostream& dump_object(std::ostream& out, boost::iterator_range<Iterator> const& range)
{
	for (auto const& any : range)
	{
		/*auto const& name =*/ any.name();
		dump(out, any.value());
	}
	return out;
}


template<typename Iterator>
std::ostream& dump(std::ostream& out, boost::iterator_range<Iterator> const& range)
{
	std::stringstream cur;
	std::stringstream fus;
	for (auto const& any : range)
		dump(out, any);
	return out;
}

std::ostream& dump(std::ostream& out, bobl::flyweight::Any const& any)
{
	auto type = bobl::bson::type(any);
	bobl::to_string(type);
	switch (type)
	{
		case bobl::Type::Dictionary:
		{
			auto object = bobl::bson::cast<bobl::flyweight::Object>(any);
			dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(object));
			break;
		}
		case bobl::Type::Array:
		{
			auto array = bobl::bson::cast<bobl::flyweight::Array>(any);
			dump(out, bobl::bson::make_iterator_range<bobl::flyweight::Any>(array));
			break;
		}
		default:
			std::stringstream cur;
			cur << "   " << cpp_type(type);
			break;
	}
	return out;
}

BOOST_AUTO_TEST_CASE(Simple1Test)
{
	//////////BSON////////////
	//{'array': [0, 1, 2, 101]}
	//(45) :
	//b'-\x00\x00\x00\x04array\x00!\x00\x00\x00\x100\x00\x00\x00\x00\x00\x101\x00\x01\x00\x00\x00\x102\x00\x02\x00\x00\x00\x103\x00e\x00\x00\x00\x00\x00'
	std::uint8_t data[] = { 0x2d, 0x0,  0x0,  0x0,  0x4,  0x61, 0x72, 0x72, 0x61,
		0x79, 0x0,  0x21, 0x0,  0x0,  0x0,  0x10, 0x30, 0x0,
		0x0,  0x0,  0x0,  0x0,  0x10, 0x31, 0x0,  0x1,  0x0,
		0x0,  0x0,  0x10, 0x32, 0x0,  0x2,  0x0,  0x0,  0x0,
		0x10, 0x33, 0x0,  0x65, 0x0,  0x0,  0x0,  0x0,  0x0 };

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
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

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
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
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
}

BOOST_AUTO_TEST_CASE(BinaryAsAnyTest)
{
	//(20):b'\x14\x00\x00\x00\x05bin\x00\x05\x00\x00\x00\x00bytes\x00'
	std::uint8_t data[] = { 0x14, 0x0, 0x0, 0x0, 0x5, 0x62, 0x69, 0x6e, 0x0, 0x5, 0x0, 0x0, 0x0, 0x0, 0x62, 0x79, 0x74, 0x65, 0x73, 0x0 };
	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
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

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
}

BOOST_AUTO_TEST_CASE(IntegerTest)
{
	auto value = Vector<long long>
	{
		{
			(std::numeric_limits<long long>::min)(),
			-1000000000000000000,
			-100000000000000000,
			-10000000000000000,
			-1000000000000000,
			-100000000000000,
			-10000000000000,
			-1000000000000,
			-100000000000,
			-10000000000,
			-4294967297,
			-100000,
			-10000,
			-1000,
			-100,
			-10,
			-1,
			-33,
			-32
			-31,
			-30
			-29,
		    -28,
		    -27,
		    -26,
		    -25,
		    -24,
		    -23,
		    -22,
		    -21,
		    -20,
		    -10,
		    -5,
		    -1,			
			0,
			1,
			5,
			10,
			20,
			21,
			22,
			23,
			24,
			25,
			26,
			27,
			28,
			29,
			30,
			31,
			32,
			33,
			100,
			1000,
			10000,
			100000,
			10000000000,
			100000000000,
			1000000000000,
			10000000000000,
			100000000000000,
			1000000000000000,
			100000000000000000,
			1000000000000000000,
			(std::numeric_limits<long long>::max)(),
		}
	};
	auto data = bobl::bson::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
}


BOOST_AUTO_TEST_CASE(IntegerOptimizeSizeTest)
{
	auto value = Vector<long long>
	{
		{
			(std::numeric_limits<long long>::min)(),
			-1000000000000000000,
			-100000000000000000,
			-10000000000000000,
			-1000000000000000,
			-100000000000000,
			-10000000000000,
			-1000000000000,
			-100000000000,
			-10000000000,
			-4294967297,
			-100000,
			-10000,
			-1000,
			-100,
			-10,
			-1,
			-33,
			-32
			-31,
			-30
			-29,
		    -28,
		    -27,
		    -26,
		    -25,
		    -24,
		    -23,
		    -22,
		    -21,
		    -20,
		    -10,
		    -5,
		    -1,			
			0,
			1,
			5,
			10,
			20,
			21,
			22,
			23,
			24,
			25,
			26,
			27,
			28,
			29,
			30,
			31,
			32,
			33,
			100,
			1000,
			10000,
			100000,
			10000000000,
			100000000000,
			1000000000000,
			10000000000000,
			100000000000000,
			1000000000000000,
			100000000000000000,
			1000000000000000000,
			(std::numeric_limits<long long>::max)(),
		}
	};
	auto data = bobl::bson::encode<bobl::options::IntegerOptimizeSize>(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	auto doc = bobl::bson::flyweight::Document::decode(begin, end);
	std::stringstream out;
	dump_object(out, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(doc));
}

BOOST_AUTO_TEST_SUITE_END()
