// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once

#include "bobl/cbor/cbor.hpp"
#include "bobl/utility/float.hpp"
#include "bobl/utility/iterator.hpp"
#include <boost/format.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/type_index.hpp>
#include <iterator>
#include <limits>
#include <cmath>
#include <cstdint>
#include <cassert>

namespace bobl{ namespace cbor {
	
enum class MajorType : std::uint8_t 
{
	UnsignedInt = Type::UnsignedInt,
	NegativeInt = Type::NegativeInt,
	ByteString = Type::ByteString,
	TextString = Type::TextString,
	Array = Type::Array,
	Dictionary = Type::Dictionary,
	Tag = Type::Tag,
	Float = Type::SimpleValue,
	SimpleValue = Type::SimpleValue,
};

enum Mask : std::uint8_t
{
	MajorTypeMask = std::uint8_t(0xff << 5),
	AditionalInfoMask = (0xff >> 3)
};

template<std::size_t>
struct TypeAdditionalInfo {};

template<> struct TypeAdditionalInfo<sizeof(std::uint8_t)> 
{
	using type = std::uint8_t;
	static constexpr std::uint8_t value = 24; 
};

template<> struct TypeAdditionalInfo<sizeof(std::uint16_t)> 
{ 
	using type = std::uint16_t;
	static constexpr std::uint8_t value = 25; 
};

template<> struct TypeAdditionalInfo<sizeof(std::uint32_t)> 
{ 
	using type = std::uint32_t;
	static constexpr std::uint8_t value = 26; 
};

template<> struct TypeAdditionalInfo<sizeof(std::uint64_t)> 
{ 
	using type = std::uint64_t;
	static constexpr std::uint8_t value = 27; 
};

inline const char* to_string(MajorType type);

namespace utility { namespace decode { 

enum:std::uint64_t { IndefiniteLength = (std::numeric_limits<std::uint64_t>::max)() };

inline constexpr bobl::cbor::Type type(std::uint8_t val) { return bobl::cbor::Type(val); }
inline constexpr bobl::cbor::MajorType major_type(bobl::cbor::Type type) { return bobl::cbor::MajorType(type&bobl::cbor::MajorTypeMask); }
inline constexpr bobl::cbor::MajorType major_type(std::uint8_t val) { return major_type(type(val)); }
inline constexpr bool is_integer(cbor::MajorType type) { return (std::uint8_t(type) & (~bobl::cbor::NegativeInt)) == 0; }
inline constexpr bool is_float(bobl::cbor::Type type) { return ((type & (cbor::SimpleValue | 24)) == (cbor::SimpleValue | 24)) && (type & 3) != 0; }

template<bobl::cbor::MajorType Type, typename Iterator>
void validate(Iterator& begin, Iterator end)
{
	if(begin == end)
		throw bobl::InputToShort{ "not enought data to decode CBOR value type" };
	auto type = bobl::cbor::utility::decode::major_type(*begin);
	if(type != Type)
		throw bobl::IncorrectObjectType{ str(boost::format("CBOR value has unexpected major type : %1% (%2$#x) insted of expected %3% (%4$#x) type") 
																							% to_string(type) % int(type) % to_string(Type) % int(Type)) };
}

template<typename Iterator>
std::uint64_t integer(Iterator& begin, Iterator end)
{
    auto res = uint64_t(type(*begin)&cbor::AditionalInfoMask);
    ++begin;
    switch(res)
    {
		case TypeAdditionalInfo<sizeof(std::uint8_t)>::value:
            res = uint64_t(boost::endian::big_to_native(bobl::utility::read<std::uint8_t>(begin, end)));
            break;
		case TypeAdditionalInfo<sizeof(std::uint16_t)>::value:
			res = uint64_t(boost::endian::big_to_native(bobl::utility::read<std::uint16_t>(begin, end)));
            break;
		case TypeAdditionalInfo<sizeof(std::uint32_t)>::value:
			res = uint64_t(boost::endian::big_to_native(bobl::utility::read<std::uint32_t>(begin, end)));
            break;
		case TypeAdditionalInfo<sizeof(std::uint64_t)>::value:
			res = uint64_t(boost::endian::big_to_native(bobl::utility::read<std::uint64_t>(begin, end)));
            break;
        case 31:
            assert(!"should be delt beforehead");
    }
    return res;
}

template<typename T, typename Iterator>
auto integer(Iterator& begin, Iterator end) -> typename std::enable_if<std::is_signed<T>::value, T>::type
{
	auto type = major_type(*begin);
	auto res = integer<typename std::make_unsigned<T>::type>(begin, end);
	return type == bobl::cbor::NegativeInt ? T(-res - 1) : T(res);
}

template<typename T, typename Iterator>
auto integer(Iterator& begin, Iterator end) -> typename std::enable_if<!std::is_signed<T>::value, T>::type
{
	assert((major_type(*begin) & (~bobl::cbor::NegativeInt)) == 0 && "must be validate beforehand");
	auto info = type(*begin)&cbor::AditionalInfoMask;
	if (info != TypeAdditionalInfo<sizeof(T)>::value)
		throw bobl::IncorrectObjectType{ str(boost::format("CBOR value has additional info : %1% insted of expected %2% ") % int(info) % int(TypeAdditionalInfo<sizeof(T)>::value)) };

	return boost::endian::big_to_native(bobl::utility::read<typename TypeAdditionalInfo<sizeof(T)>::type>(++begin, end));
}

template<typename T, typename Iterator>
std::uint64_t tag(Iterator& begin, Iterator end)
{
	validate<cbor::MajorType::Tag>(begin, end);
	return integer(begin, end);
}

template<std::uint8_t Tag, typename Iterator>
void validate_tag(Iterator& begin, Iterator end)
{
	validate<cbor::MajorType::Tag>(begin, end);
	auto tag = integer(begin, end);
	if (tag != Tag)
		throw bobl::IncorrectObjectType{ str(boost::format("CBOR value has invalid tag : %1% (%1$#x) insted of expected %2% (%2$#x) type") % tag % Tag) };
}


template<typename T, typename Iterator>
T floating_point(Iterator& begin, Iterator end)
{
	T res = 0;
	static_assert(std::is_floating_point<T>::value, "T expected to be floating point type");
    auto subtype =*begin&cbor::AditionalInfoMask;
    ++begin;
    switch(subtype)
    {
        case TypeAdditionalInfo<sizeof(std::uint16_t)>::value:
		{
			auto value = bobl::utility::FloatConverter<sizeof(std::uint16_t)>{}(boost::endian::big_to_native(bobl::utility::read<std::uint16_t>(begin, end)));
			assert(!(value > (std::numeric_limits<T>::max)() || value < (std::numeric_limits<T>::lowest)()));
			res = T(value);
			break;
		}
		case TypeAdditionalInfo<sizeof(std::uint32_t)>::value:
		{
			auto value = bobl::utility::FloatConverter<sizeof(std::uint32_t)>{}(boost::endian::big_to_native(bobl::utility::read<std::uint32_t>(begin, end)));
			if (value > (std::numeric_limits<T>::max)() || value < (std::numeric_limits<T>::lowest)())
				throw bobl::Overflow{ str(boost::format("%1% is too short to hold %2%") % boost::typeindex::type_id<T>().pretty_name() % value) };
			res = T(value);
			break;
		}
		case TypeAdditionalInfo<sizeof(std::uint64_t)>::value:
		{
			auto value = bobl::utility::FloatConverter<sizeof(std::uint64_t)>{}(boost::endian::big_to_native(bobl::utility::read<std::uint64_t>(begin, end)));
			if (value >(std::numeric_limits<T>::max)() || value < (std::numeric_limits<T>::lowest)())
				throw bobl::Overflow{ str(boost::format("%1% is too short to hold %2%") % boost::typeindex::type_id<T>().pretty_name() % value) };
			res = T(value);
			break;
		}
		default:
			throw bobl::IncorrectObjectType{ str(boost::format("CBOR floating point type has unexpected sub-type : %1%") % int(subtype)) };

    }
    return res;
}
template<typename Iterator>
std::uint64_t length(Iterator& begin, Iterator end)
{
	if ((type(*begin)&cbor::AditionalInfoMask) == 31)
	{
		++begin;
		return bobl::cbor::utility::decode::IndefiniteLength;
	}
	auto len = integer(begin, end);
	if (len == bobl::cbor::utility::decode::IndefiniteLength)
		throw bobl::InvalidObject{"CBOR: to many object to decode" };
	return len;
}

template<typename T, typename Iterator>
T string(Iterator& begin, Iterator end)
{
	auto len = bobl::cbor::utility::decode::length(begin, end);
	if (len == bobl::cbor::utility::decode::IndefiniteLength)
	{
		auto res = T{};
		for (;;)
		{
			if (begin == end)
				throw bobl::InvalidObject{ "not enough data provided to decode indefinite length CBOR string" };
			if (*begin == cbor::Break)
				break;
			res.push_back(*begin++);
		}
		return res;
	}

	auto first = begin;
	if (decltype(len)(std::distance(begin, end)) < len)
		throw bobl::InvalidObject{ "not enough data provided to decode CBOR string" };
	std::advance(begin, len);
	return { first, begin };
}

} /*namespace decode*/ } /*namespace utility */

namespace utility { namespace encode {

namespace workaround
{
template<typename T, size_t SizeOf = sizeof(T)> struct ResolveAmbiguity { using type = T; };
template<typename T> struct ResolveAmbiguity<T, sizeof(std::uint8_t)>
{
	using type = typename std::conditional<std::is_signed<T>::value, std::int8_t, std::uint8_t>::type;
};

template<typename T> struct ResolveAmbiguity<T, sizeof(std::uint16_t)>
{
	using type = typename std::conditional<std::is_signed<T>::value, std::int16_t, std::uint16_t>::type;
};

template<typename T> struct ResolveAmbiguity<T, sizeof(std::uint32_t)>
{
	using type = typename std::conditional<std::is_signed<T>::value, std::int32_t, std::uint32_t>::type;
};

template<typename T> struct ResolveAmbiguity<T, sizeof(std::uint64_t)> 
{ 
	using type = typename std::conditional<std::is_signed<T>::value, std::int64_t, std::uint64_t>::type;
};
}

template<typename Iterator, typename T>
Iterator unsigned_int_strict(Iterator out, cbor::MajorType type, T value)
{
	static_assert(std::is_integral<T>::value && !std::is_signed<T>::value, "T expected to be unsigned integral type");
	//boost::endian::native_to_big_inplace(value);
	value = T(boost::endian::native_to_big<typename workaround::ResolveAmbiguity<T>::type >(value));
	auto begin = reinterpret_cast<std::uint8_t const*>(&value);
	auto end = begin + sizeof(T) / sizeof(std::uint8_t);
	out = std::uint8_t(type) | TypeAdditionalInfo<sizeof(T)>::value;
	return std::copy(begin, end, ++out);

}

template<typename Iterator, typename T>
Iterator unsigned_int(Iterator out, cbor::MajorType type, T value)
{
	static_assert(std::is_integral<T>::value && !std::is_signed<T>::value, "T expected to be unsigned integral type");
	if (value < 24)
	{
		out = std::uint8_t(std::uint8_t(value) | std::uint8_t(type));
		++out;
	}
	else
	{
		if (value > (std::numeric_limits<std::uint8_t>::max)())
		{
			if (value > (std::numeric_limits<std::uint16_t>::max)())
				out = value > (std::numeric_limits<std::uint32_t>::max)()
						? unsigned_int_strict(out, type, value)
						: unsigned_int_strict(out, type, std::uint32_t(value));
			else
				out = unsigned_int_strict(out, type, std::uint16_t(value));
		}
		else
			out = unsigned_int_strict(out, type, std::uint8_t(value));
	}
	return out;
}

} /*namespace encode*/ } /*namespace utility */

inline const char* to_string(MajorType type)
{
	const char* res = "unknown type";
	switch (type)
	{
		case MajorType::UnsignedInt:
			res = "unsigned int";
			break;
		case MajorType::NegativeInt:
			res = "negative int";
			break;
		case MajorType::ByteString:
			res = "byte string";
			break;
		case MajorType::TextString:
			res = "text string";
			break;
		case MajorType::Array:
			res = "array";
			break;
		case MajorType::Dictionary:
			res = "dictionary";
			break;
		case MajorType::Tag:
			res = "tag";
			break;
		case MajorType::SimpleValue:
			res = "simple value/float";
			break;
	}
	return res;
}

inline const char* to_string(Type type)
{
	auto major = utility::decode::major_type(type);
	if (major != MajorType::SimpleValue)
		return to_string(major);
	if (type == Type::Break)
		return "Break";

	if (utility::decode::is_float(type))
		type = Type(type & 27);

	const char* res = "unknown type";
	switch (type)
	{
		case Type::False:
			res = "false";
			break;
		case Type::True:
			res = "true";
			break;
		case Type::Null:
			res = "null";
			break;
		case Type::Float16:
			res = "float 16";
			break;
		case Type::Float32:
			res = "float 32";
			break;
		case Type::Float64:
			res = "float 64";
			break;
		default:
			assert(!"unexpected CBOR type");
			break;
	}
	return res;
}


} /*namespace cbor*/ } /*namespace bobl*/