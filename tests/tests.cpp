#define BOOST_TEST_MODULE BOBLTests

#include "tests.hpp"
//#include "bobl/any.hpp"
#include "bobl/bson/details/options.hpp"
#include "bobl/cbor/details/options.hpp"
#include "bobl/json/details/options.hpp"
#include "bobl/utility/adapter.hpp"
#include "bobl/adapter.hpp"
#include "bobl/utility/parameters.hpp"
#include "bobl/utility/decoders.hpp"
#include "bobl/utility/names.hpp"
#include "bobl/utility/has_is.hpp"
#include "bobl/utility/options.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/options.hpp"
#include "bobl/names.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/back.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/fusion/include/size.hpp>
#include <tuple>
#include <string>
#include <type_traits>

namespace bobl {
	template<typename Options>
	class MemberName<DisabledNames, int, 1, Options> {};
} /*namespace bobl*/


BOOST_AUTO_TEST_SUITE(BOBLTestSuite)

void options_compiletime_test()
{
	using O1 = bobl::Options<bobl::options::ByteType<char>>;
	static_assert(bobl::utility::options::Contains<O1, bobl::options::ByteType<char>>::value, "...");
	using O2 = bobl::Options<O1>;
	static_assert(bobl::utility::options::Contains<O2, bobl::options::ByteType<char>>::value, "bobl::utility::options::Contains not working on nested options");
	using O3 = bobl::Options<O2>;
	static_assert(bobl::utility::options::Contains<O3, bobl::options::ByteType<char>>::value, "bobl::utility::options::Contains not working on nested options");
}

BOOST_AUTO_TEST_CASE(OptionsCompileTimeTest)
{
	options_compiletime_test();
}

void decode_parameters_compiletime_test()
{
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, bobl::Options<bobl::options::StructAsDictionary>>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, bobl::Options<bobl::options::StructAsDictionary>>::Options, bobl::Options<bobl::options::StructAsDictionary>>::value, "seems bobl::utility::DecodeParameters is broken");


	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, std::tuple<int>>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, std::tuple<int>>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, bobl::options::None>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, bobl::options::None>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, std::tuple<int>, bobl::options::None>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, std::tuple<int>, bobl::options::None>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");


	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, float>::Parameters, std::tuple<int, float>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, float>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, float, bobl::Options<bobl::options::StructAsDictionary>>::Parameters, std::tuple<int, float>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, int, float, bobl::Options<bobl::options::StructAsDictionary>>::Options, bobl::Options<bobl::options::StructAsDictionary>>::value, "seems bobl::utility::DecodeParameters is broken");


	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, bobl::Options<bobl::options::StructAsDictionary>>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, bobl::Options<bobl::options::StructAsDictionary>>::Options, bobl::Options<bobl::options::StructAsDictionary>>::value, "seems bobl::utility::DecodeParameters is broken");


	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, std::tuple<int>>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, std::tuple<int>>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, bobl::options::None>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, bobl::options::None>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, std::tuple<int>, bobl::options::None>::Parameters, std::tuple<int>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, std::tuple<int>, bobl::options::None>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");


	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, float>::Parameters, std::tuple<int, float>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, float>::Options, bobl::options::None>::value, "seems bobl::utility::DecodeParameters is broken");

	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, float, bobl::Options<bobl::options::StructAsDictionary>>::Parameters, std::tuple<int, float>>::value, "seems bobl::utility::DecodeParameters is broken");
	static_assert(std::is_same<typename bobl::utility::DecodeParameters<bobl::bson::NsTag, int, float, bobl::Options<bobl::options::StructAsDictionary>>::Options, bobl::Options<bobl::options::StructAsDictionary>>::value, "seems bobl::utility::DecodeParameters is broken");

}


void adapter_compiletime_test()
{
	static_assert(std::is_same<bobl::Adapter<TheEnum>::type, std::underlying_type<TheEnum>::type>::value, "seems bobl::Adapter doesn't work as expected");
	static_assert(!bobl::utility::Adaptable<int, bobl::Adapter<int>>::value, "seems bobl::utility::utility::decode::Adaptable doesn't work as expected");
	static_assert(bobl::utility::Adaptable<TheEnum, bobl::Adapter<TheEnum>>::value, "seems bobl::utility::utility::decode::Adaptable doesn't work as expected");
}

BOOST_AUTO_TEST_CASE(UtilityCompileTimeTest)
{
	decode_parameters_compiletime_test();
}

void named_sequence_compiletime_test()
{
	static_assert(bobl::utility::NamedSequence<IntDictionary, bobl::options::None>::value, "seems bobl::utility::NamedSequence<IntDictionary> doesn't work as expected");
	static_assert(!bobl::utility::NamedSequence<DisabledNames, bobl::options::None>::value, "seems bobl::utility::NamedSequence<DisabledNames> doesn't work as expected");
	static_assert(!bobl::utility::NamedSequence<std::tuple<int, std::string>, bobl::options::None>::value, "seems bobl::utility::NamedSequence<std::tuple<...>> doesn't work as expected");
	static_assert(bobl::utility::NamedSequence<std::tuple<int, std::string>, bobl::options::UsePositionAsName>::value, "seems bobl::utility::NamedSequence<std::tuple<...>> ignoring bobl::options::UsePositionAsName");
	static_assert(bobl::utility::NamedSequence<std::tuple<diversion::variant<bobl::UseTypeName, int>>, bobl::options::None>::value, "seems bobl::utility::NamedSequence<std::tuple<variant<bobl::UseTypeName,...>>> doesn't work as expected");
	static_assert(!bobl::utility::NamedSequence<std::tuple<diversion::variant<bobl::UseTypeName, int>, int>, bobl::options::None>::value, "seems bobl::utility::NamedSequence<std::tuple<variant<bobl::UseTypeName,...>>> doesn't work as expected");
	static_assert(!bobl::utility::NamedSequence<std::tuple<diversion::variant<float, int>>, bobl::options::None>::value, "seems bobl::utility::NamedSequence<std::tuple<variant<bobl::UseTypeName,...>>> doesn't work as expected");


	static_assert(!std::is_same<
							typename bobl::utility::GetNameType<bobl::MemberName<SimpleTuple, typename std::tuple_element<0, SimpleTuple>::type, 0, bobl::options::None>>::type,
							bobl::utility::ObjectNameIrrelevant
					>::value, "name type is expected");	
	static_assert(!std::is_same<
							typename bobl::utility::GetNameType<bobl::MemberName<SimpleTuple, typename std::tuple_element<1, SimpleTuple>::type, 1, bobl::options::None>>::type,
							bobl::utility::ObjectNameIrrelevant
					>::value, "name type is expected");	
	static_assert(!std::is_same<
							typename bobl::utility::GetNameType<bobl::MemberName<SimpleTuple, typename std::tuple_element<2, SimpleTuple>::type, 2, bobl::options::None>>::type,
							bobl::utility::ObjectNameIrrelevant
					>::value, "name type is expected");	
	static_assert(!std::is_same<
							typename bobl::utility::GetNameType<bobl::MemberName<SimpleTuple, typename std::tuple_element<3, SimpleTuple>::type, 3, bobl::options::None>>::type,
							bobl::utility::ObjectNameIrrelevant
					>::value, "name type is expected");	

	static_assert(bobl::utility::NamedSequence<SimpleTuple, bobl::options::None>::value, "not supposed to be named sequence");
}

BOOST_AUTO_TEST_CASE(MemberNameTest)
{
	named_sequence_compiletime_test();
	using MemberName = typename bobl::utility::GetNameType<bobl::MemberName<Simple, TheEnum, 3, bobl::options::None>>::type;
	BOOST_CHECK_EQUAL(MemberName{}.name(),"theEnum");
	BOOST_CHECK(MemberName{}.compare(std::string{ "theEnum" }));

	{
		auto name = bobl::MemberName<SimpleTuple, typename std::tuple_element<0, SimpleTuple>::type, 0, bobl::options::None>{}();
		BOOST_CHECK_EQUAL(std::string{ "enabled" }, std::string{ name });
	}
	{
		auto name = bobl::MemberName<SimpleTuple, typename std::tuple_element<1, SimpleTuple>::type, 1, bobl::options::None>{}();
		BOOST_CHECK_EQUAL(std::string{ "id" }, std::string{ name });
	}
	{
		auto name = bobl::MemberName<SimpleTuple, typename std::tuple_element<2, SimpleTuple>::type, 2, bobl::options::None>{}();
		BOOST_CHECK_EQUAL(std::string{ "name" }, std::string{ name });
	}
	{
		auto name = bobl::MemberName<SimpleTuple, typename std::tuple_element<3, SimpleTuple>::type, 3, bobl::options::None>{}();
		BOOST_CHECK_EQUAL(std::string{ "theEnum" }, std::string{ name });
	}
}


void map_decoder_compiletime_test()
{
	static_assert(bobl::utility::DictionaryDecoderCompatible<IntDictionary, bobl::options::None>::value, "seems bobl::utility::DictionaryDecoderCompatible<IntDictionary> doesn't work as expected");
	static_assert(!bobl::utility::DictionaryDecoderCompatible<DisabledNames, bobl::options::None>::value, "seems bobl::utility::DictionaryDecoderCompatible<DisabledNames> doesn't work as expected");
	static_assert(!bobl::utility::DictionaryDecoderCompatible<int, bobl::options::None>::value, "seems bobl::utility::DictionaryDecoderCompatible<int> doesn't work as expected");


	static_assert(boost::mpl::and_<boost::fusion::traits::is_sequence<IntDictionary>,
							bobl::utility::DictionaryDecoderCompatible<IntDictionary, bobl::options::None>>::value, "seems bobl::utility::DictionaryDecoderCompatible<IntDictionary> based decoder selector doesn't work as expected");

	static_assert(!boost::mpl::and_<boost::fusion::traits::is_sequence<IntDictionary>,
						boost::mpl::not_<bobl::utility::DictionaryDecoderCompatible<IntDictionary, bobl::options::None>>>::value, "seems bobl::utility::DictionaryDecoderCompatible<IntDictionary> based decoder selector  doesn't work as expected");

	static_assert(!boost::mpl::and_<boost::fusion::traits::is_sequence<IntDictionary>,
						bobl::utility::DictionaryDecoderCompatible<DisabledNames, bobl::options::None>>::value, "seems bobl::utility::DictionaryDecoderCompatible<DisabledNames> based decoder selector  doesn't work as expected");

	static_assert(boost::mpl::and_<boost::fusion::traits::is_sequence<IntDictionary>,
					boost::mpl::not_<bobl::utility::DictionaryDecoderCompatible<DisabledNames, bobl::options::None>>>::value, "seems bobl::utility::DictionaryDecoderCompatible<DisabledNames>based decoder selector  doesn't work as expected");

}

BOOST_AUTO_TEST_CASE(MapDecoderTest)
{
	map_decoder_compiletime_test();
}

//template<typename ...Args>
//void test_xxx();
//
//void any_compiletime_test()
//{
//	test_xxx<bobl::details::X::ConstPointersRange>();
//	test_xxx<typename bobl::details::MakeVariant<bobl::details::X::ConstPointersRange>::type>();
//	test_xxx<bobl::flyweight::Any>::type>();
//}
//
//BOOST_AUTO_TEST_CASE(AnyTest)
//{
//	any_compiletime_test();
//}


BOOST_AUTO_TEST_CASE(EnumAddapterTest)
{
//     enum X { x0, x1, x2, x3 };
//     static_assert(bobl::utility::adapters::Adapt<X >::value, "oops can't adapt enum");
}



BOOST_AUTO_TEST_SUITE_END()
