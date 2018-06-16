// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <type_traits>
#include <cstdint>

namespace bobl{ namespace bson {

enum Type : std::uint8_t
{
	Double = 0x01,
	Utf8String = 0x02,
	EmbeddedDocument = 0x03,
	Array = 0x04,
	Binary = 0x05,
    Bool = 0x08, 
	UTCDateTime = 0x09,
    Int32 = 0x10,
	Timestamp = 0x11,
	Int64 = 0x12,
	Null = 0x0a,
	DBPointer = 0x0c,
	JavaScript = 0x0d,
	Symbol = 0x0e,
	MinKey = 0xFF,
	MaxKey = 0x7F
};

template<Type T>
struct SizeOf { using type = int; };

template<>
struct SizeOf<Double> { using type = std::integral_constant<std::size_t, sizeof(std::uint64_t)>; };

template<>
struct SizeOf<Bool>{ using type = std::integral_constant<std::size_t, sizeof(std::uint8_t)>; };

template<>
struct SizeOf<Int32> { using type = std::integral_constant<std::size_t, sizeof(std::uint32_t)>; };

template<>
struct SizeOf<Int64> { using type =  std::integral_constant<std::size_t, sizeof(std::uint64_t)>; };

template<>
struct SizeOf<UTCDateTime > { using type = std::integral_constant<std::size_t, sizeof(std::uint64_t)>; };

template<>
struct SizeOf<Timestamp> { using type = std::integral_constant<std::size_t, sizeof(std::uint64_t)>; };

template<>
struct SizeOf<Null> { using type = std::integral_constant<std::size_t, 0>; };

template<>
struct SizeOf<MinKey>{ using type =  std::integral_constant<std::size_t, 0>; };

template<>
struct SizeOf<MaxKey> { using type =  std::integral_constant<std::size_t, 0>; };

namespace utility {

template<typename T>
struct FixedSize : std::false_type {};

template<typename T, T N>
struct FixedSize<std::integral_constant<T, N>> : std::true_type {};

} // namespace utility

enum BinSubtype
{
	Generic = 0x00,
	Function = 0x01,
	BinaryOld = 0x02,
	UuidOld = 0x03,
	Uuid = 0x04,
	MD5 = 0x05,
	UserDefined = 0x80
};

// template<Type type>
// struct TypeTraits{};
// 
// template<Type type>
// struct TypeTraits 
// {
//     static std::size_t SizeOf() {}
// };



inline const char* to_string(Type type)
{
	const char* res = "unknown type";
	switch (type)
	{
		case Type::Double:
			res = "Double";
			break;
		case Type::Utf8String:
			res = "Utf8String";
			break;
		case Type::EmbeddedDocument:
			res = "Embedded document";
			break;
		case Type::Array:
			res = "Array";
			break;
		case Type::Binary:
			res = "Binary";
			break;
        case Type::Bool:
            res = "Bool";
            break;
		case Type::UTCDateTime:
			res = "UTCDateTime";
			break;
        case Type::Int32:
			res = "Int32";
			break;
		case Type::Timestamp:
			res = "Timestamp";
			break;
		case Type::Int64:
			res = "Int64";
			break;
		case Type::Null:
			res = "Null";
			break;
		case Type::DBPointer:
			res = "DBPointer";
			break;
		case Type::JavaScript:
			res = "JavaScript";
			break;
		case Type::Symbol:
			res = "Symbol";
			break;
		case Type::MinKey:
			res = "MinKey";
			break;
		case Type::MaxKey:
			res = "MaxKey";
			break;
	}
	return res;
}

inline const char* to_string(BinSubtype subtype)
{
	const char* res = "unknown subtype";
	switch (subtype)
	{
		case Generic:
			res = "Generic";
		break;
		case Function:
			res = "Function";
		break;
		case BinaryOld:
			res = "Binary (old)";
		case UuidOld:
			res = "UUID (old)";
		break;
		case Uuid:
			res = "UUID";
		break;
		case MD5:
			res = "MD5";
		break;
		case UserDefined:
			res = "user defined";
		break;
	}
	return res;
}

}/*namespace bson*/ } /*namespace bobl*/