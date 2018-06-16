#include <boost/test/unit_test.hpp>
#include "bobl/cbor/cast.hpp"
#include "bobl/cbor/decode.hpp"
#include "tests.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include<tuple>
#include<string>
#include <algorithm>
#include <iterator>
#include <cstdint>


BOOST_AUTO_TEST_SUITE(BOBL_CBOR_TestSuite)


BOOST_AUTO_TEST_CASE(OptionalTest)
{
	//{'array': [0, 1, 2, 101]}
	//(13) : b'\xa1earray\x84\x00\x01\x02\x18e'
	std::uint8_t data[] = { /*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x84, 0x0, 0x1, 0x2, 0x18, 0x65 };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK(!(bobl::cbor::decode<diversion::optional<int>>(begin, end)));
	BOOST_CHECK_EQUAL(begin, data);
	BOOST_CHECK(!(bobl::cbor::decode<diversion::optional<Simple>>(begin, end)));
	BOOST_CHECK_EQUAL(begin, data);
	auto resx = bobl::cbor::decode<diversion::optional<std::vector<int>>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(!resx, false);
	auto& res = resx.get();
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(res[0], 0);
	BOOST_CHECK_EQUAL(res[1], 1);
	BOOST_CHECK_EQUAL(res[2], 2);
	BOOST_CHECK_EQUAL(res[3], 101);
	BOOST_CHECK(!(bobl::cbor::decode<diversion::optional<int>, bobl::options::None>(begin, end)));
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK(!(bobl::cbor::decode<diversion::optional<Simple>, bobl::options::None>(begin, end)));
	BOOST_CHECK_EQUAL(begin, end);
}


BOOST_AUTO_TEST_CASE(SimpleTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(38) : b'\xa4genabled\xf5bid\x18ddnamehthe namegtheEnum\x02'
	std::uint8_t data[] = { 0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0xf5, 0x62, 0x69, 
							0x64, 0x18, 0x64, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x68, 0x74, 0x68, 0x65, 
							0x20, 0x6e, 0x61, 0x6d, 0x65, 0x67, 0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 
							0x6d, 0x2 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto simple = bobl::cbor::decode<Simple, bobl::options::None>(begin, end);
	BOOST_CHECK(simple.enabled);
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(simple.name, std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
	BOOST_CHECK_EQUAL(begin, end);
}


BOOST_AUTO_TEST_CASE(SimpleOptioonalTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(38) : b'\xa4genabled\xf5bid\x18ddnamehthe namegtheEnum\x02'
	std::uint8_t data[] = { 0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0xf5, 0x62, 0x69, 
							0x64, 0x18, 0x64, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x68, 0x74, 0x68, 0x65, 
							0x20, 0x6e, 0x61, 0x6d, 0x65, 0x67, 0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 
							0x6d, 0x2 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto simple = bobl::cbor::decode<SimpleOptional>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);

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
}


BOOST_AUTO_TEST_CASE(SimpleXTest)
{
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(38) : b'\xa4genabled\xf5bid\x18ddnamehthe namegtheEnum\x02'
	std::uint8_t data[] = { 0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0xf5, 0x62, 0x69, 
							0x64, 0x18, 0x64, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x68, 0x74, 0x68, 0x65, 
							0x20, 0x6e, 0x61, 0x6d, 0x65, 0x67, 0x74, 0x68, 0x65, 0x45, 0x6e, 0x75, 
							0x6d, 0x2 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto simple = bobl::cbor::decode<SimpleVariant>(begin, end);
	BOOST_CHECK(boost::get<bool>(simple.enabled));
	BOOST_CHECK_EQUAL(simple.id, 100);
	BOOST_CHECK_EQUAL(boost::get<std::string>(simple.name), std::string{ "the name" });
	BOOST_CHECK_EQUAL(int(boost::get<TheEnumClass>(simple.theEnum)), 2);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(IntArrayTest)
{
	//{'array': [0, 1, 2, 101]}
	//(13) : b'\xa1earray\x84\x00\x01\x02\x18e'
	std::uint8_t data[] = { /*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x84, 0x0, 0x1, 0x2, 0x18, 0x65 };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<std::vector<int>>(begin, end);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(res[0], 0);
	BOOST_CHECK_EQUAL(res[1], 1);
	BOOST_CHECK_EQUAL(res[2], 2);
	BOOST_CHECK_EQUAL(res[3], 101);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(IntArrayAsAnyArrayTest)
{
	//{'array': [0, 1, 2, 101]}
	//(13) : b'\xa1earray\x84\x00\x01\x02\x18e'
	std::uint8_t data[] = { /*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x84, 0x0, 0x1, 0x2, 0x18, 0x65 };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<std::vector<bobl::flyweight::lite::Any<decltype(begin)>>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(res[0]), 0);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(res[1]), 1);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(res[2]), 2);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<int>(res[3]), 101);
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
							0x40, 0x59, 0x47, 0xe6, 0xb6, 0xee, 0x3e, 0x37};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<std::vector<double>>(begin, end);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(res[0], 0.1);
	BOOST_CHECK_EQUAL(res[1], 5555.77777);
	BOOST_CHECK_EQUAL(res[2], 2.1);
	BOOST_CHECK_EQUAL(res[3], 101.1234567);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(FloatArrayAsAnyArrayTest)
{
	//{'array': [0.1, 5555.77777, 2.1, 101.1234567]}
	//(44) :
	//b'\xa1earray\x84\xfb?\xb9\x99\x99\x99\x99\x99\x9a\xfb@\xb5\xb3\xc7\x1b\xefI\xcf\xfb@\x00\xcc\xcc\xcc\xcc\xcc\xcd\xfb@YG\xe6\xb6\xee>7'
	std::uint8_t data[] = {/*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x84, 0xfb,
							0x3f, 0xb9, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9a, 0xfb,
							0x40, 0xb5, 0xb3, 0xc7, 0x1b, 0xef, 0x49, 0xcf, 0xfb,
							0x40, 0x0,  0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcd, 0xfb,
							0x40, 0x59, 0x47, 0xe6, 0xb6, 0xee, 0x3e, 0x37};

	uint8_t const *begin = data;
	uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<std::vector<bobl::flyweight::lite::Any<decltype(begin)>>>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.size(), 4);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<double>(res[0]), 0.1);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<double>(res[1]), 5555.77777);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<double>(res[2]), 2.1);
	BOOST_CHECK_EQUAL(bobl::cbor::cast<double>(res[3]), 101.1234567);
}


BOOST_AUTO_TEST_CASE(ArraysOfSimpleTest)
{
  //{'array': [{'enabled': True, 'id': 10, 'name': 'first', 'theEnum': 1},
  //{'enabled': False, 'id': 20, 'name': 'second', 'theEnum': 2}, {'enabled':
  //True, 'id': 30, 'name': 'third', 'theEnum': 3}]} (112) :
  //b'\xa1earray\x83\xa4genabled\xf5bid\ndnameefirstgtheEnum\x01\xa4genabled\xf4bid\x14dnamefsecondgtheEnum\x02\xa4genabled\xf5bid\x18\x1ednameethirdgtheEnum\x03'
  std::uint8_t data[] = {
						  /*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x83, 0xa4, 0x67, 0x65, 0x6e,
						  0x61, 0x62, 0x6c, 0x65, 0x64, 0xf5, 0x62, 0x69, 0x64, 0xa,  0x64, 0x6e,
						  0x61, 0x6d, 0x65, 0x65, 0x66, 0x69, 0x72, 0x73, 0x74, 0x67, 0x74, 0x68,
						  0x65, 0x45, 0x6e, 0x75, 0x6d, 0x1,  0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62,
						  0x6c, 0x65, 0x64, 0xf4, 0x62, 0x69, 0x64, 0x14, 0x64, 0x6e, 0x61, 0x6d,
						  0x65, 0x66, 0x73, 0x65, 0x63, 0x6f, 0x6e, 0x64, 0x67, 0x74, 0x68, 0x65,
						  0x45, 0x6e, 0x75, 0x6d, 0x2,  0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c,
						  0x65, 0x64, 0xf5, 0x62, 0x69, 0x64, 0x18, 0x1e, 0x64, 0x6e, 0x61, 0x6d,
						  0x65, 0x65, 0x74, 0x68, 0x69, 0x72, 0x64, 0x67, 0x74, 0x68, 0x65, 0x45,
						  0x6e, 0x75, 0x6d, 0x3};

  uint8_t const *begin = data;
  uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
  auto res = bobl::cbor::decode<std::vector<Simple>>(begin, end);
  BOOST_CHECK_EQUAL(res.size(), 3);
  BOOST_CHECK(res[0].enabled);
  BOOST_CHECK_EQUAL(res[0].id, 10);
  BOOST_CHECK_EQUAL(res[0].name, std::string{ "first" });
  BOOST_CHECK_EQUAL(int(res[0].theEnum), 1);

  BOOST_CHECK(!res[1].enabled);
  BOOST_CHECK_EQUAL(res[1].id, 20);
  BOOST_CHECK_EQUAL(res[1].name, std::string{ "second" });
  BOOST_CHECK_EQUAL(int(res[1].theEnum), 2);

  BOOST_CHECK(res[2].enabled);
  BOOST_CHECK_EQUAL(res[2].id, 30);
  BOOST_CHECK_EQUAL(res[2].name, std::string{ "third" });
  BOOST_CHECK_EQUAL(int(res[2].theEnum), 3);


  BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(ArraysOfSimpleAsAnyTest)
{
  //{'array': [{'enabled': True, 'id': 10, 'name': 'first', 'theEnum': 1},
  //{'enabled': False, 'id': 20, 'name': 'second', 'theEnum': 2}, 
  //{'enabled':True, 'id': 30, 'name': 'third', 'theEnum': 3}]} 
  //(112) :b'\xa1earray\x83\xa4genabled\xf5bid\ndnameefirstgtheEnum\x01\xa4genabled\xf4bid\x14dnamefsecondgtheEnum\x02\xa4genabled\xf5bid\x18\x1ednameethirdgtheEnum\x03'
  std::uint8_t data[] = {
						  /*0xa1, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,*/ 0x83, 0xa4, 0x67, 0x65, 0x6e,
						  0x61, 0x62, 0x6c, 0x65, 0x64, 0xf5, 0x62, 0x69, 0x64, 0xa,  0x64, 0x6e,
						  0x61, 0x6d, 0x65, 0x65, 0x66, 0x69, 0x72, 0x73, 0x74, 0x67, 0x74, 0x68,
						  0x65, 0x45, 0x6e, 0x75, 0x6d, 0x1,  0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62,
						  0x6c, 0x65, 0x64, 0xf4, 0x62, 0x69, 0x64, 0x14, 0x64, 0x6e, 0x61, 0x6d,
						  0x65, 0x66, 0x73, 0x65, 0x63, 0x6f, 0x6e, 0x64, 0x67, 0x74, 0x68, 0x65,
						  0x45, 0x6e, 0x75, 0x6d, 0x2,  0xa4, 0x67, 0x65, 0x6e, 0x61, 0x62, 0x6c,
						  0x65, 0x64, 0xf5, 0x62, 0x69, 0x64, 0x18, 0x1e, 0x64, 0x6e, 0x61, 0x6d,
						  0x65, 0x65, 0x74, 0x68, 0x69, 0x72, 0x64, 0x67, 0x74, 0x68, 0x65, 0x45,
						  0x6e, 0x75, 0x6d, 0x3};

  uint8_t const *begin = data;
  uint8_t const *end = begin + sizeof(data) / sizeof(data[0]);
  auto res = bobl::cbor::decode<std::vector<bobl::flyweight::lite::Any<decltype(begin)>>>(begin, end);
  BOOST_CHECK_EQUAL(begin, end);
  BOOST_CHECK_EQUAL(res.size(), 3);
  {
	  auto simple = bobl::cbor::cast<Simple>(res[0]);
	  BOOST_CHECK(simple.enabled);
	  BOOST_CHECK_EQUAL(simple.id, 10);
	  BOOST_CHECK_EQUAL(simple.name, std::string{ "first" });
	  BOOST_CHECK_EQUAL(int(simple.theEnum), 1);
  }
  {
	  auto simple = bobl::cbor::cast<Simple>(res[1]);
	  BOOST_CHECK(!simple.enabled);
	  BOOST_CHECK_EQUAL(simple.id, 20);
	  BOOST_CHECK_EQUAL(simple.name, std::string{ "second" });
	  BOOST_CHECK_EQUAL(int(simple.theEnum), 2);
  }
  {
	  auto simple = bobl::cbor::cast<Simple>(res[2]);
	  BOOST_CHECK(simple.enabled);
	  BOOST_CHECK_EQUAL(simple.id, 30);
	  BOOST_CHECK_EQUAL(simple.name, std::string{ "third" });
	  BOOST_CHECK_EQUAL(int(simple.theEnum), 3);
  }
}

BOOST_AUTO_TEST_CASE(DictionaryOfIntegersEncodedInDifferentOrderTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
	//(83) : b'\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfd'
	std::uint8_t data[] = { 0xa8, 0x64, 0x74, 0x69, 0x6e, 0x79, 0x3, 0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x80, 0x64, 0x69, 0x6e, 0x74, 0x5f, 0x19, 0x7f, 0xff, 0x65, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x1a, 0xf, 0xff, 0xff, 0xff, 0x68, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x23, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x38, 0x78, 0x67, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x39, 0x7f, 0xfc, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x3a, 0xf, 0xff, 0xff, 0xfd };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<IntDictionary, bobl::Options<bobl::options::StructAsDictionary>>(begin, end);
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
	BOOST_CHECK_EQUAL(res.int_, 32767);
	BOOST_CHECK_EQUAL(res.int64, 268435455);
	BOOST_CHECK_EQUAL(res.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.neg_short, -121);
	BOOST_CHECK_EQUAL(res.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(DictionaryOfIntegersEncodedInSameOrderTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
	//(83) : b'\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfd'
	std::uint8_t data[] = { 0xa8, 0x64, 0x74, 0x69, 0x6e, 0x79, 0x3, 0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x80, 0x64, 0x69, 0x6e, 0x74, 0x5f, 0x19, 0x7f, 0xff, 0x65, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x1a, 0xf, 0xff, 0xff, 0xff, 0x68, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x23, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x38, 0x78, 0x67, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x39, 0x7f, 0xfc, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x3a, 0xf, 0xff, 0xff, 0xfd };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<OrderedIntDictionary, bobl::Options<bobl::options::StructAsDictionary>>(begin, end);
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
	BOOST_CHECK_EQUAL(res.int_, 32767);
	BOOST_CHECK_EQUAL(res.int64, 268435455);
	BOOST_CHECK_EQUAL(res.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.neg_short, -121);
	BOOST_CHECK_EQUAL(res.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(DictionaryOfIntegersAndMoreTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
	//(83) : b'\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfd'
	std::uint8_t data[] = { 0xa8, 0x64, 0x74, 0x69, 0x6e, 0x79, 0x3, 0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x80, 0x64, 0x69, 0x6e, 0x74, 0x5f, 0x19, 0x7f, 0xff, 0x65, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x1a, 0xf, 0xff, 0xff, 0xff, 0x68, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x23, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x38, 0x78, 0x67, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x39, 0x7f, 0xfc, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x3a, 0xf, 0xff, 0xff, 0xfd };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<IntDictionaryX, bobl::Options<bobl::options::StructAsDictionary>>(begin, end);
	BOOST_CHECK(res.xtra1.empty());
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
	BOOST_CHECK_EQUAL(res.int_, 32767);
	BOOST_CHECK_EQUAL(res.int64, 268435455);
	BOOST_CHECK_EQUAL(res.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.neg_short, -121);
	BOOST_CHECK_EQUAL(res.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
	BOOST_CHECK(res.xtra2.empty());
	BOOST_CHECK(res.xtra3.empty());
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(StructOfIntegersEncodedInSameOrderTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
	//(83) : b'\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfd'
	std::uint8_t data[] = { 0xa8, 0x64, 0x74, 0x69, 0x6e, 0x79, 0x3, 0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x80, 0x64, 0x69, 0x6e, 0x74, 0x5f, 0x19, 0x7f, 0xff, 0x65, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x1a, 0xf, 0xff, 0xff, 0xff, 0x68, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x23, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x38, 0x78, 0x67, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x39, 0x7f, 0xfc, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x3a, 0xf, 0xff, 0xff, 0xfd };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<OrderedIntDictionary>(begin, end);
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
	BOOST_CHECK_EQUAL(res.int_, 32767);
	BOOST_CHECK_EQUAL(res.int64, 268435455);
	BOOST_CHECK_EQUAL(res.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.neg_short, -121);
	BOOST_CHECK_EQUAL(res.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.neg_int64, -268435454);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(ShortStructOfIntegersEncodedInSameOrderTest)
{
	//{'tiny': 3, 'short_': 128, 'int_': 32767, 'int64': 268435455, 'neg_tiny': -4, 'neg_short': -121, 'neg_int': -32765, 'neg_int64': -268435454}
	//(83) : b'\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfd'
	std::uint8_t data[] = { 0xa8, 0x64, 0x74, 0x69, 0x6e, 0x79, 0x3, 0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0x18, 0x80, 0x64, 0x69, 0x6e, 0x74, 0x5f, 0x19, 0x7f, 0xff, 0x65, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x1a, 0xf, 0xff, 0xff, 0xff, 0x68, 0x6e, 0x65, 0x67, 0x5f, 0x74, 0x69, 0x6e, 0x79, 0x23, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x38, 0x78, 0x67, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x39, 0x7f, 0xfc, 0x69, 0x6e, 0x65, 0x67, 0x5f, 0x69, 0x6e, 0x74, 0x36, 0x34, 0x3a, 0xf, 0xff, 0xff, 0xfd };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<ShortOrderedIntDictionary>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.tiny, 3);
	BOOST_CHECK_EQUAL(res.short_, 128);
}

BOOST_AUTO_TEST_CASE(OrderedFlotDictionaryTest, *boost::unit_test::tolerance(0.00001))
{
	//////////CBOR////////////
	//{'nul': 0.0, 'short_': 3.333, 'short_max': 65504.0, 'f32': 32767.0, 'f64': 2147483650.99999, 'neg_short': -3.555}
	//(94) : b'\xa6cnul\xfb\x00\x00\x00\x00\x00\x00\x00\x00fshort_\xfb@\n\xa9\xfb\xe7l\x8bDishort_max\xfb@\xef\xfc\x00\x00\x00\x00\x00cf32\xfb@\xdf\xff\xc0\x00\x00\x00\x00cf64\xfbA\xe0\x00\x00\x00_\xff\xebineg_short\xfb\xc0\x0cp\xa3\xd7\n=q'
    std::uint8_t data[] = {
					0xa6, 0x63, 0x6e, 0x75, 0x6c, 0xfb, 0x0,  0x0,  0x0,  0x0,  0x0,
					0x0,  0x0,  0x0,  0x66, 0x73, 0x68, 0x6f, 0x72, 0x74, 0x5f, 0xfb,
					0x40, 0xa,  0xa9, 0xfb, 0xe7, 0x6c, 0x8b, 0x44, 0x69, 0x73, 0x68,
					0x6f, 0x72, 0x74, 0x5f, 0x6d, 0x61, 0x78, 0xfb, 0x40, 0xef, 0xfc,
					0x0,  0x0,  0x0,  0x0,  0x0,  0x63, 0x66, 0x33, 0x32, 0xfb, 0x40,
					0xdf, 0xff, 0xc0, 0x0,  0x0,  0x0,  0x0,  0x63, 0x66, 0x36, 0x34,
					0xfb, 0x41, 0xe0, 0x0,  0x0,  0x0,  0x5f, 0xff, 0xeb, 0x69, 0x6e,
					0x65, 0x67, 0x5f, 0x73, 0x68, 0x6f, 0x72, 0x74, 0xfb, 0xc0, 0xc,
					0x70, 0xa3, 0xd7, 0xa,  0x3d, 0x71};
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<OrderedFlotDictionary>(begin, end);
	BOOST_CHECK_EQUAL(begin, end);
	BOOST_CHECK_EQUAL(res.nul, 0);
	BOOST_TEST(res.short_ == 3.333);
	BOOST_CHECK_EQUAL(res.short_max, 65504.0);
	BOOST_CHECK_EQUAL(res.f32, 32767);
	BOOST_CHECK_EQUAL(res.f64, 2147483650.99999);
	BOOST_TEST(res.neg_short == -3.555);
}

BOOST_AUTO_TEST_CASE(NestedTest)
{
  // {'name': 'name of something', 'ints' : {'tiny': 3, 'short_' : 128, 'int_' :
  // 32767, 'int64' : 268435455, 'neg_tiny' : -4, 'neg_short' : -121, 'neg_int'
  // : -32765, 'neg_int64' : -268435454}, 'level1' : {'name': 'level1', 'short_'
  // : 123, 'level2' : {'name': 'level2', 'short_' : -255, 'level3' : {'name':
  // 'level3', 'short_' : -1}}}}
  //(198) : b'\xa3dnameqname of
  //somethingdints\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfdflevel1\xa3dnameflevel1fshort_\x18{flevel2\xa3dnameflevel2fshort_8\xfeflevel3\xa2dnameflevel3fshort_
  //'
	std::uint8_t data[] = {	0xa3, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x71, 0x6e, 0x61, 0x6d, 0x65, 0x20,
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
							0x68, 0x6f, 0x72, 0x74, 0x5f, 0x20};

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<Nested<IntDictionary>, bobl::Options<bobl::options::StructAsDictionary>>(begin, end);
	BOOST_CHECK_EQUAL(res.name, std::string{ "name of something" });
	BOOST_CHECK_EQUAL(res.ints.tiny, 3);
	BOOST_CHECK_EQUAL(res.ints.short_, 128);
	BOOST_CHECK_EQUAL(res.ints.int_, 32767);
	BOOST_CHECK_EQUAL(res.ints.int64, 268435455);
	BOOST_CHECK_EQUAL(res.ints.neg_tiny, -4);
	BOOST_CHECK_EQUAL(res.ints.neg_short, -121);
	BOOST_CHECK_EQUAL(res.ints.neg_int, -32765);
	BOOST_CHECK_EQUAL(res.ints.neg_int64, -268435454);
	BOOST_CHECK_EQUAL(res.level1.name, std::string{ "level1" });
	BOOST_CHECK_EQUAL(res.level1.short_, 123);
	BOOST_CHECK_EQUAL(res.level1.level2.name, std::string{ "level2" });
	BOOST_CHECK_EQUAL(res.level1.level2.short_, -255);
	BOOST_CHECK_EQUAL(res.level1.level2.level3.name, std::string{ "level3" });
	BOOST_CHECK_EQUAL(res.level1.level2.level3.short_, -1);
	BOOST_CHECK_EQUAL(begin, end);
}

BOOST_AUTO_TEST_CASE(NestedOrderedIntTest)
{
  // {'name': 'name of something', 'ints' : {'tiny': 3, 'short_' : 128, 'int_' :
  // 32767, 'int64' : 268435455, 'neg_tiny' : -4, 'neg_short' : -121, 'neg_int'
  // : -32765, 'neg_int64' : -268435454}, 'level1' : {'name': 'level1', 'short_'
  // : 123, 'level2' : {'name': 'level2', 'short_' : -255, 'level3' : {'name':
  // 'level3', 'short_' : -1}}}}
  //(198) : b'\xa3dnameqname of
  //somethingdints\xa8dtiny\x03fshort_\x18\x80dint_\x19\x7f\xffeint64\x1a\x0f\xff\xff\xffhneg_tiny#ineg_short8xgneg_int9\x7f\xfcineg_int64:\x0f\xff\xff\xfdflevel1\xa3dnameflevel1fshort_\x18{flevel2\xa3dnameflevel2fshort_8\xfeflevel3\xa2dnameflevel3fshort_
  //'
	std::uint8_t data[] = {	0xa3, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x71, 0x6e, 0x61, 0x6d, 0x65, 0x20,
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
		0x68, 0x6f, 0x72, 0x74, 0x5f, 0x20};

		uint8_t const* begin = data;
		uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
		auto res = bobl::cbor::decode<Nested<OrderedIntDictionary>>(begin, end);
		BOOST_CHECK_EQUAL(res.name, std::string{ "name of something" });
		BOOST_CHECK_EQUAL(res.ints.tiny, 3);
		BOOST_CHECK_EQUAL(res.ints.short_, 128);
		BOOST_CHECK_EQUAL(res.ints.int_, 32767);
		BOOST_CHECK_EQUAL(res.ints.int64, 268435455);
		BOOST_CHECK_EQUAL(res.ints.neg_tiny, -4);
		BOOST_CHECK_EQUAL(res.ints.neg_short, -121);
		BOOST_CHECK_EQUAL(res.ints.neg_int, -32765);
		BOOST_CHECK_EQUAL(res.ints.neg_int64, -268435454);
		BOOST_CHECK_EQUAL(res.level1.name, std::string{ "level1" });
		BOOST_CHECK_EQUAL(res.level1.short_, 123);
		BOOST_CHECK_EQUAL(res.level1.level2.name, std::string{ "level2" });
		BOOST_CHECK_EQUAL(res.level1.level2.short_, -255);
		BOOST_CHECK_EQUAL(res.level1.level2.level3.name, std::string{ "level3" });
		BOOST_CHECK_EQUAL(res.level1.level2.level3.short_, -1);
		BOOST_CHECK_EQUAL(begin, end);

}


BOOST_AUTO_TEST_CASE(ByteStringTest)
{
	//(11):b'\xa1cbinEbytes'
	std::uint8_t data[] = { /*0xa1, 0x63, 0x62, 0x69, 0x6e,*/ 0x45, 0x62, 0x79, 0x74, 0x65, 0x73 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::cbor::decode<std::vector<uint8_t>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL(res.size(), 5);
	BOOST_CHECK_EQUAL(res[0], 0x62);
	BOOST_CHECK_EQUAL(res[1], 0x79);
	BOOST_CHECK_EQUAL(res[2], 0x74);
	BOOST_CHECK_EQUAL(res[3], 0x65);
	BOOST_CHECK_EQUAL(res[4], 0x73);
}

BOOST_AUTO_TEST_CASE(ByteStringCharTest)
{
	//(11):b'\xa1cbinEbytes'
	std::uint8_t data[] = { /*0xa1, 0x63, 0x62, 0x69, 0x6e,*/ 0x45, 0x62, 0x79, 0x74, 0x65, 0x73 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK_THROW((bobl::cbor::decode<std::vector<char>>(begin, end)), bobl::IncorrectObjectType);

	/*uint8_t const**/ begin = data;
	auto expected = std::string{ "bytes" };
	auto res = bobl::cbor::decode<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(expected), std::end(expected));
}

BOOST_AUTO_TEST_CASE(BinaryAsAnyTest)
{
	//(11):b'\xa1cbinEbytes'
	std::uint8_t data[] = { /*0xa1, 0x63, 0x62, 0x69, 0x6e,*/ 0x45, 0x62, 0x79, 0x74, 0x65, 0x73 };
	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);

	auto any = bobl::cbor::decode<bobl::flyweight::lite::Any<decltype(begin)>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_THROW((bobl::cbor::cast<std::vector<char>>(any)), bobl::IncorrectObjectType);
	auto res = bobl::cbor::cast<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(any);
	auto expected = std::string{ "bytes" };
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res), std::end(res), std::begin(expected), std::end(expected));
}

BOOST_AUTO_TEST_CASE(UUIDVectorTest2)
{
	//(47):b'\xa1fvector\x82\xd8%P\xea\xa8\x99v\xcf\xccC\xde\xa2;\x10\xc6{\x10\n\xe8\xd8%P\x9d\x89H$7/N\xf7\xa1xy7\xf7\x05G\x83'
	std::uint8_t data[] = {
				0xa1, 0x66, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x82, 0xd8, 0x25, 0x50,
				0xea, 0xa8, 0x99, 0x76, 0xcf, 0xcc, 0x43, 0xde, 0xa2, 0x3b, 0x10, 0xc6,
				0x7b, 0x10, 0xa,  0xe8, 0xd8, 0x25, 0x50, 0x9d, 0x89, 0x48, 0x24, 0x37,
				0x2f, 0x4e, 0xf7, 0xa1, 0x78, 0x79, 0x37, 0xf7, 0x5,  0x47, 0x83};

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	BOOST_CHECK_THROW((bobl::cbor::decode<Vector<int>>(begin, end)), bobl::IncorrectObjectType);

	boost::uuids::string_generator gen;
	auto expected = std::vector<boost::uuids::uuid>{ gen("EAA89976-CFCC-43DE-A23B-10C67B100AE8"), gen("9D894824-372F-4EF7-A178-7937F7054783") };
	/*uint8_t const**/ begin = data;
	auto res = bobl::cbor::decode<Vector<boost::uuids::uuid>>(begin, end);
	BOOST_CHECK(begin == end);
	BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(res.vector), std::end(res.vector), std::begin(expected), std::end(expected));
}



BOOST_AUTO_TEST_SUITE_END()
