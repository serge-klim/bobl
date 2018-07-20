#include <boost/test/unit_test.hpp>
#include "tests.hpp"
#include "bobl/cbor/iterator.hpp"
#include "bobl/cbor/cast.hpp"
#include "bobl/cbor/decode.hpp"
#include "bobl/cbor/encode.hpp"
#include "bobl/bobl.hpp"
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <ostream>
#include <sstream>
#include <cstdint>


BOOST_AUTO_TEST_SUITE(BOBL_CBOR_Iterators_TestSuite)

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

std::string dump(bobl::Type type, bobl::flyweight::Any const& any)
{
	std::stringstream out;
	switch (type)
	{
		case bobl::Type::Bool:
			out<<bobl::cbor::cast<bool>(any);
			break; 
		case bobl::Type::Tiny:
		case bobl::Type::Int8:
			out << bobl::cbor::cast<std::int8_t>(any);
			break;
		case bobl::Type::Int16:
			out << bobl::cbor::cast<std::int16_t>(any);
			break;
		case bobl::Type::Int32:
			out << bobl::cbor::cast<std::int32_t>(any);
			break;
		case bobl::Type::Int64:
			out << bobl::cbor::cast<std::int64_t>(any);
			break;
		case bobl::Type::UnsignedInt8:
			out << bobl::cbor::cast<std::uint8_t>(any);
			break;
		case bobl::Type::UnsignedInt16:
			out << bobl::cbor::cast<std::uint16_t>(any);
			break;
		case bobl::Type::UnsignedInt32:
			out << bobl::cbor::cast<std::uint32_t>(any);
			break;
		case bobl::Type::UnsignedInt64:
			out << bobl::cbor::cast<std::uint64_t>(any);
			break;
		case bobl::Type::Float8:
		case bobl::Type::Float16:
		case bobl::Type::Float32:
			out << bobl::cbor::cast<float>(any);
			break;
		case bobl::Type::Float64:
			out << bobl::cbor::cast<double>(any);
			break;
		case bobl::Type::String:
			out << bobl::cbor::cast<std::string>(any);
			break;
		case bobl::Type::Binary:
			bobl::cbor::cast<std::vector<std::uint8_t>>(any);
			out << "<binary>";
			break; 
		case bobl::Type::Timepoint:
			bobl::cbor::cast<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>>(any);
			out << "<timepoint>";
			break;
		case bobl::Type::UUID:
			bobl::cbor::cast<boost::uuids::uuid>(any);
			out << "<uuid>";
			break;
		default:
			throw std::runtime_error(str(boost::format("unsupported type %1% (%2$#x)") % to_string(type) % int(type)));
			break;
	}
	return out.str();
}

std::ostream& dump(std::ostream& out, bobl::flyweight::Any const& any);

template<typename Iterator>
std::ostream& dump_object(std::ostream& out, boost::iterator_range<Iterator> const& range)
{
	for (auto const& any : range)
	{
		/*auto const& name = */any.name();
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
	auto type = bobl::cbor::type(any);
	bobl::to_string(type);
	switch (type)
	{
		case bobl::Type::Dictionary:
		{
			auto object = bobl::cbor::cast<bobl::flyweight::Object>(any);
			dump_object(out, bobl::cbor::make_iterator_range<bobl::flyweight::NameValue<bobl::flyweight::Any>>(object));
			break;
		}
		case bobl::Type::Array:
		{
			auto array = bobl::cbor::cast<bobl::flyweight::Array>(any);
			dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(array));
			break;
		}
		default:
			std::stringstream cur;
			cur << "   " << cpp_type(type) << " : " << dump(type, any);
			break;
	}
	return out;
}

BOOST_AUTO_TEST_CASE(Simple1Test)
{
	std::uint8_t data[] = { 0x81, 0x7b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	std::stringstream out;
	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_CASE(Sample1Test)
{
	std::uint8_t data[] = {
		0xa1, 0x77, 0x4a, 0x53, 0x4f, 0x4e, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20,
		0x50, 0x61, 0x74, 0x74, 0x65, 0x72, 0x6e, 0x20, 0x70, 0x61, 0x73, 0x73,
		0x33, 0xa2, 0x6c, 0x49, 0x6e, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x74,
		0x65, 0x73, 0x74, 0x70, 0x49, 0x74, 0x20, 0x69, 0x73, 0x20, 0x61, 0x6e,
		0x20, 0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x2e, 0x73, 0x54, 0x68, 0x65,
		0x20, 0x6f, 0x75, 0x74, 0x65, 0x72, 0x6d, 0x6f, 0x73, 0x74, 0x20, 0x76,
		0x61, 0x6c, 0x75, 0x65, 0x78, 0x1b, 0x6d, 0x75, 0x73, 0x74, 0x20, 0x62,
		0x65, 0x20, 0x61, 0x6e, 0x20, 0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x20,
		0x6f, 0x72, 0x20, 0x61, 0x72, 0x72, 0x61, 0x79, 0x2e };

	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_CASE(IntArrayTest)
{
	std::uint8_t data[] =
	{
		0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79, 0x84, 0xfb,
		0x3f, 0xb9, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9a, 0xfb,
		0x40, 0xb5, 0xb3, 0xc7, 0x1b, 0xef, 0x49, 0xcf, 0xfb,
		0x40, 0x0,  0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcd, 0xfb,
		0x40, 0x59, 0x47, 0xe6, 0xb6, 0xee, 0x3e, 0x37
	};

	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_CASE(ArraysOfSimpleTest)
{
	std::uint8_t data[] =
	{
		0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79, 0x83, 0xa4, 0x67, 0x65, 0x6e,
		0x61, 0x62, 0x6c, 0x65, 0x64, 0xf5, 0x62, 0x69, 0x64, 0xa,  0x64, 0x6e,
		0x61, 0x6d, 0x65, 0x65, 0x66, 0x69, 0x72, 0x73, 0x74, 0x67, 0x74, 0x68,
		0x65, 0x45, 0x6e, 0x75, 0x6d, 0x1,  0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62,
		0x6c, 0x65, 0x64, 0xf4, 0x62, 0x69, 0x64, 0x14, 0x64, 0x6e, 0x61, 0x6d,
		0x65, 0x66, 0x73, 0x65, 0x63, 0x6f, 0x6e, 0x64, 0x67, 0x74, 0x68, 0x65,
		0x45, 0x6e, 0x75, 0x6d, 0x2,  0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c,
		0x65, 0x64, 0xf5, 0x62, 0x69, 0x64, 0x18, 0x1e, 0x64, 0x6e, 0x61, 0x6d,
		0x65, 0x65, 0x74, 0x68, 0x69, 0x72, 0x64, 0x67, 0x74, 0x68, 0x65, 0x45,
		0x6e, 0x75, 0x6d, 0x3
	};

	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_CASE(NestedIntTest)
{
	// {'name': 'name of something', 'ints' : {'tiny': 3, 'short_' : 128, 'int_' :
	// 32767, 'int64' : 268435455, 'neg_tiny' : -4, 'neg_short' : -121, 'neg_int'
	// : -32765, 'neg_int64' : -268435454}, 'level1' : {'name': 'level1', 'short_'
	// : 123, 'level2' : {'name': 'level2', 'short_' : -255, 'level3' : {'name':
	// 'level3', 'short_' : -1}}}}
	//(198) : b'\xa3dnameqname of
	//somethingdints\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfdflevel1\xa3dnameflevel1fshort_\x18{flevel2\xa3dnameflevel2fshort_8\xfeflevel3\xa2dnameflevel3fshort_
	//'
	std::uint8_t data[] = { 0xa3, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x71, 0x6e, 0x61, 0x6d, 0x65, 0x20,
		0x6f, 0x66, 0x20, 0x73, 0x6f, 0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67,
		0x64, 0x69, 0x6e, 0x74, 0x73, 0xa8, 0x64, 0x74, 0x69, 0x6e, 0x79, 0x3,
		0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x80, 0x64, 0x69, 0x6e,
		0x74, 0x5f, 0x19, 0x7f, 0xff, 0x65, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x1a,
		0xf,  0xff, 0xff, 0xff, 0x68, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e,
		0x79, 0x23, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74,
		0x38, 0x78, 0x67, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x39, 0x7f,
		0xfc, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x3a,
		0xf,  0xff, 0xff, 0xfd, 0x66, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x31, 0xa3,
		0x64, 0x6e, 0x61, 0x6d, 0x65, 0x66, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x31,
		0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x7b, 0x66, 0x6c, 0x65,
		0x76, 0x65, 0x6c, 0x32, 0xa3, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x66, 0x6c,
		0x65, 0x76, 0x65, 0x6c, 0x32, 0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f,
		0x38, 0xfe, 0x66, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x33, 0xa2, 0x64, 0x6e,
		0x61, 0x6d, 0x65, 0x66, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x33, 0x66, 0x73,
		0x68, 0x6f, 0x72, 0x74, 0x5f, 0x20 };

	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}
BOOST_AUTO_TEST_CASE(ByteStringTest)
{
	//(11):b'\xa1cbinEbytes'
	std::uint8_t data[] = { 0xa1, 0x63, 0x62, 0x69, 0x6e, 0x45, 0x62, 0x79, 0x74, 0x65, 0x73 };
	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_CASE(UUIDVectorTest2)
{
	//(47):b'\xa1fvector\x82\xd8%P\xea\xa8\x99v\xcf\xccC\xde\xa2;\x10\xc6{\x10\n\xe8\xd8%P\x9d\x89H$7/N\xf7\xa1xy7\xf7\x05G\x83'
	std::uint8_t data[] = {
		0xa1, 0x66, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x82, 0xd8, 0x25, 0x50,
		0xea, 0xa8, 0x99, 0x76, 0xcf, 0xcc, 0x43, 0xde, 0xa2, 0x3b, 0x10, 0xc6,
		0x7b, 0x10, 0xa,  0xe8, 0xd8, 0x25, 0x50, 0x9d, 0x89, 0x48, 0x24, 0x37,
		0x2f, 0x4e, 0xf7, 0xa1, 0x78, 0x79, 0x37, 0xf7, 0x5,  0x47, 0x83 };

	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_CASE(FloatArrayTest)
{
	//{'array': [0.1, 5555.77777, 2.1, 101.1234567]}
	//(44) :
	//b'\xa1earray\x84\xfb?\xb9\x99\x99\x99\x99\x99\x9a\xfb@\xb5\xb3\xc7\x1b\xefI\xcf\xfb@\x00\xcc\xcc\xcc\xcc\xcc\xcd\xfb@YG\xe6\xb6\xee>7'
	std::uint8_t data[] = {/*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x84, 0xfb,
		0x3f, 0xb9, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9a, 0xfb,
		0x40, 0xb5, 0xb3, 0xc7, 0x1b, 0xef, 0x49, 0xcf, 0xfb,
		0x40, 0x0,  0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcd, 0xfb,
		0x40, 0x59, 0x47, 0xe6, 0xb6, 0xee, 0x3e, 0x37 };

	auto begin = data;
	auto end = begin + sizeof(data) / sizeof(data[0]);
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
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
	auto data = bobl::cbor::encode(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
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
	auto data = bobl::cbor::encode<bobl::options::IntegerOptimizeSize>(value);
	uint8_t const* begin = data.data();
	uint8_t const* end = begin + data.size();
	std::stringstream out;
	dump(out, bobl::cbor::make_iterator_range<bobl::flyweight::Any>(begin, end));
}

BOOST_AUTO_TEST_SUITE_END()
