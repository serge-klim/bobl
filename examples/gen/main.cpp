#include "bobl/bson/iterator.hpp"
#include "bobl/bson/flyweight.hpp"
#include "bobl/bson/cast.hpp"
#include "bobl/bson/decode.hpp"
#include "bobl/bobl.hpp"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/format.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdint>

using Any = bobl::flyweight::lite::Any<bobl::bson::flyweight::Iterator>;
using NameValue = bobl::flyweight::NameValue<Any>;

//std::string cpp_type(bobl::bson::Type type)
//{
//	auto res = std::string{ "bobl::flyweight::lite::Any<bobl::bson::flyweight::Iterator>" };
//	switch (type)
//	{
//		case bobl::bson::Double:
//			res = "double";
//			break;
//		case bobl::bson::Utf8String:
//			res = "std::string";
//			break;
//		case bobl::bson::Binary:
//			res = "std::vector<std::uint8_t>";
//			break; 
//		case bobl::bson::Bool:
//			res = "std::string";
//			break; 
//		case bobl::bson::UTCDateTime:
//			res = "std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>";
//			break; 
//		case bobl::bson::Int32:
//			res = "int";
//			break; 
//		case bobl::bson::Int64:
//			res = "long long";
//			break;
//		default:
////			throw std::runtime_error(str(boost::format("unsupported type %1% (%2$#x)") % to_string(type) %type));
//			break;
//	}
//	return res;
//}

std::string cpp_type(bobl::Type type)
{
	auto res = std::string{ "bobl::flyweight::lite::Any<bobl::bson::flyweight::Iterator>" };
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

template<typename Iterator>
std::string array_type(std::ostream& out, boost::iterator_range<Iterator> const& range)
{
	auto type = bobl::Type::Array;
	if (!range.empty())
	{
		auto i = range.begin();
		auto first_type = bobl::bson::type(*i);
		switch (type)
		{
			case bobl::Type::Dictionary:
				break;
			case bobl::Type::Array:
				break;
			default:
			{
				i = std::find_if(++i, range.end(), [first_type](auto const& value)
				{
					return bobl::bson::type(value) != first_type;
				});
				if (i == range.end())
					type = first_type;
				break;
			}
		}
	}
	return str(boost::format("vector<%1%>") % cpp_type(type));
}

template<typename Iterator>
std::ostream& dump(std::ostream& out, std::string name, boost::iterator_range<Iterator> const& range)
{
	std::stringstream cur;
	std::stringstream fus;
	cur << "struct struct_" << name <<"\n{\n";
	fus << "BOOST_FUSION_ADAPT_STRUCT(\n   struct_" << name << ",\n";
	for (auto const& any : range)
	{
		auto const& name = any.name();
		auto const& value = any.value();
		auto type = bobl::bson::type(value);
		fus << "   " << name << ",\n";
		switch (type)
		{
			case bobl::Type::Dictionary:
			{
				cur << "   struct_" <<name;
				auto object = bobl::bson::cast<bobl::flyweight::lite::Object<bobl::bson::flyweight::Iterator>>(value);
				dump(out, name, bobl::bson::make_iterator_range<bobl::flyweight::NameValue<Any>>(object));
				break;
			}
			case bobl::Type::Array:
			{
				auto array = bobl::bson::cast<bobl::flyweight::lite::Array<bobl::bson::flyweight::Iterator>>(value);
				cur << "   " << array_type(out, bobl::bson::make_iterator_range<Any>(array));
				break;
			}
			default:
				cur << "   " << cpp_type(type);
				break;
		}
		cur << ' ' << name << ";\n";
	}
	cur << "};\n";
	fus << ")\n";
	out << cur.str() << '\n' << fus.str() << '\n';
	return out;
}


int main(int argc, char *argv[])
{
	try
	{
		if (argc == 1)
		{
			std::cerr << "please specify BSON encoded file" << std::endl;
			return 1;
		}

		boost::iostreams::mapped_file_source mmap{ argv[1] };
		auto begin = mmap.begin();
		auto doc = bobl::bson::flyweight::Document::decode(begin,mmap.end());
		auto& out = std::cout;
		out << "#include <vector>\n"
		<< "#include <string>\n"
		<< "#include <chrono>\n"
		<< "#include <cstdint>\n"
		<< "#include <boost/fusion/include/adapt_struct.hpp>\n"
		<< "#include <boost/uuid/uuid.hpp>\n\n";
		dump(out, "Top", bobl::bson::make_iterator_range<NameValue>(doc));
	}
	catch (std::exception& e)
	{
		std::cerr << "error : " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "miserable failure" << std::endl;
	}

	return 0;
}
