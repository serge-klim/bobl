#include "struct.hpp"
#include "bobl/bson/encode.hpp"
#include "bobl/cbor/encode.hpp"
#include "bobl/bson/decode.hpp"
#include "bobl/cbor/decode.hpp"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/chrono/thread_clock.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <string>
#include <vector>
#include <iterator>
#include <fstream>
#include <iostream>
#include <cstdint>

enum class InputType{bson, cbor};

std::istream& operator>> (std::istream &in, InputType& type)
{
	struct Types : boost::spirit::qi::symbols<char, InputType >
	{
		Types()
		{
			add
				("bson", InputType::bson)
				("cbor", InputType::cbor)
				;
		}
	} static const types;

	std::string token;
	in >> token;
	auto begin = std::begin(token);
	auto end = std::end(token);
	if (!boost::spirit::qi::parse(begin, end, boost::spirit::ascii::no_case[types], type) || begin != end)
		throw std::runtime_error{ str(boost::format("unsupported encoding %1%") % token) };
	return in;
}

int main(int argc, char *argv[])
{
	try
	{
        auto description = boost::program_options::options_description{ "options" };
        description.add_options()
					("help,h", "print usage message")
					("input-type,t", boost::program_options::value<InputType>(), "input file type (bson, cbor)")
					("filename", boost::program_options::value<std::string>(), "encoded file")
					("repeat,n", boost::program_options::value<size_t>()->default_value(10000), "repeat tests")
					;

		boost::program_options::positional_options_description positional;
		positional.add("filename", -1);

        boost::program_options::variables_map vm;
        boost::program_options::store(
									boost::program_options::command_line_parser(argc, argv).
									options(description).
									positional(positional).run(), vm);
        boost::program_options::notify(vm);
        if (vm.count("help") || !vm.count("filename") || !vm.count("input-type"))
        {
            std::cout << "test \n" << description;
            return 0;
        }
		std::basic_ifstream<std::uint8_t> in(vm["filename"].as<std::string>().c_str(), std::ios::binary);
		std::vector<std::uint8_t> bson_in{ (std::istreambuf_iterator<std::uint8_t>(in)), std::istreambuf_iterator<std::uint8_t>() };
		std::vector<std::uint8_t> cbor_in;
		switch (vm["input-type"].as<InputType>())
		{
			case InputType::bson:
			{
				auto begin = bson_in.data();
				auto data = bobl::bson::decode<struct_Top>(begin, begin + bson_in.size());
				cbor_in = bobl::cbor::encode(data);
				break;
			}
			case InputType::cbor:
			{
				auto begin = bson_in.data();
				auto data = bobl::cbor::decode<struct_Top>(begin, begin + bson_in.size());
				cbor_in = bobl::bson::encode(data);
				cbor_in.swap(bson_in);
				break;
			}
		}
		auto begin = bson_in.data();
		auto data = bobl::bson::decode<struct_Top>(begin, begin + bson_in.size());

		size_t n = vm["repeat"].as<size_t>();
		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
			{
				auto begin = bson_in.data();
				bobl::bson::decode<struct_Top>(begin, begin + bson_in.size());
			}
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "bson decode (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed <<')' << std::endl;;
		}
		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
			{
				auto begin = bson_in.data();
				bobl::bson::decode<struct_Top, bobl::Options<bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers, bobl::options::RelaxedFloats>>(begin, begin + bson_in.size());
			}
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "bson decode <bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers, bobl::options::RelaxedFloats> (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}

		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
				bobl::bson::encode(data);
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "bson encode (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}

		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
				bobl::bson::encode<bobl::options::IntegerOptimizeSize>(data);
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "bson encode <bobl::options::IntegerOptimizeSize> (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}
		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
			{
				auto const* begin = cbor_in.data();
				bobl::cbor::decode<struct_Top, bobl::Options<bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers, bobl::options::RelaxedFloats>>(begin, begin + cbor_in.size());
			}
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "cbor decode <bobl::options::StructAsDictionary, bobl::options::RelaxedIntegers, bobl::options::RelaxedFloats> (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}

		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
			{
				auto const* begin = cbor_in.data();
				bobl::cbor::decode<struct_Top>(begin, begin + cbor_in.size());
			}
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "cbor decode (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}
		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
				bobl::cbor::encode(data);
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "cbor encode (" << n <<" times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}
		{
			auto start = boost::chrono::thread_clock::now();
			for (size_t i = 0; i != n; ++i)
				bobl::cbor::encode<bobl::options::IntegerOptimizeSize>(data);
			auto elapsed = boost::chrono::thread_clock::now() - start;
			std::cout << "cbor encode <bobl::options::IntegerOptimizeSize> (" << n << " times) :" << boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed) << " (" << elapsed << ')' << std::endl;;
		}

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
