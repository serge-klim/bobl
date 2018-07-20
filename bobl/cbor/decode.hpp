// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/cbor/details/decoder.hpp"
#include "bobl/cbor/details/options.hpp"
#include "bobl/cbor/details/utility.hpp"
#include "bobl/cbor/cbor.hpp"
#include "bobl/utility/flyweight.hpp"
#include "bobl/utility/parameters.hpp"
#include <type_traits>

namespace bobl{ namespace cbor { 

template<typename Iterator>
Type native_type(bobl::flyweight::lite::Any<Iterator> const& any)
{
	auto end = bobl::flyweight::lite::utility::details::end_raw(any);
	for (auto begin = bobl::flyweight::lite::utility::details::begin_raw(any); begin != end; ++begin)
	{
		auto type = bobl::cbor::utility::decode::type(*begin);
		if (type != cbor::Tag)
		{
			auto mtype = bobl::cbor::utility::decode::major_type(type);
			if (mtype != bobl::cbor::MajorType::SimpleValue)
				return bobl::cbor::Type(mtype);
			return 	utility::decode::is_float(type) ? Type(type & 27) : type;
		}
	}
	throw bobl::InputToShort{ "not enought data to decode CBOR value type from lite::Any" };
}

template<typename Iterator>
bobl::Type type(bobl::flyweight::lite::Any<Iterator> const& any)
{
	auto begin = bobl::flyweight::lite::utility::details::begin_raw(any);
	auto end = bobl::flyweight::lite::utility::details::end_raw(any);
	if(begin == end)
		throw bobl::InputToShort{ "not enought data to decode CBOR value type from lite::Any" };

	auto res = bobl::Type::Null;
	switch (auto type = utility::decode::type(*begin))
	{
		case Type::UnsignedInt8:
			res = bobl::Type::UnsignedInt8;
			break;
		case Type::UnsignedInt16:
			res = bobl::Type::UnsignedInt16;
			break;
		case Type::UnsignedInt32:
			res = bobl::Type::UnsignedInt32;
			break;
		case Type::UnsignedInt64:
			res = bobl::Type::UnsignedInt64;
			break;
		case Type::NegativeInt8:
			res = bobl::Type::Int16;
			break;
		case Type::NegativeInt16:
			res = bobl::Type::Int32;
			break;
		case Type::NegativeInt32:
			res = bobl::Type::Int64;
			break;
		case Type::NegativeInt64:
			res = bobl::Type::Int64;
			break;
		case Type::Null:
			res = bobl::Type::Null;
			break;
		case Type::False:
		case Type::True:
			res = bobl::Type::Bool;
			break;
		case Type::Float16:
			res = bobl::Type::Float16;
			break;
		case Type::Float32:
			res = bobl::Type::Float16;
			break;
		case Type::Float64:
			res = bobl::Type::Float16;
			break;
		default:
		{
			switch (auto major_type = utility::decode::major_type(type))
			{
				case MajorType::UnsignedInt:
					assert((type&cbor::AditionalInfoMask) < 24);
					res = bobl::Type::Tiny;
					break;
				case MajorType::NegativeInt:
					assert((type&cbor::AditionalInfoMask) < 24);
					res = bobl::Type::Tiny;
					break;
				case MajorType::ByteString:
					res = bobl::Type::Binary;
					break;
				case MajorType::TextString:
					res = bobl::Type::String;
					break;
				case MajorType::Array:
					res = bobl::Type::Array;
					break;
				case MajorType::Dictionary:
					res = bobl::Type::Dictionary;
					break;
				case MajorType::Tag:
					switch (auto tag = utility::decode::integer(begin, end))
					{
						case DataTimeString:
						case DataTimeNumerical:
							res = bobl::Type::Timepoint;
							break;
						case UUID:
							res = bobl::Type::UUID;
							break;
					}
					break;
				default:
					throw bobl::TypeNotSupported(str(boost::format("unsupported type %1% (%2$#x)") % to_string(type) % int(type)));
					break;
			}
		}
	}
	return res;
}

template<typename ...Args, typename Iterator>
auto decode(Iterator& begin, Iterator end) -> typename bobl::utility::DecodeParameters<bobl::cbor::NsTag, Args...>::Result
{
	using Parameters = bobl::utility::DecodeParameters<bobl::cbor::NsTag, Args...>;
	using Decoder = typename decoder::details::Decoder<typename Parameters::Result, typename Parameters::Options>::type;
	return Decoder::decode(begin, end);
}

}/*namespace cbor*/ } /*namespace bobl*/