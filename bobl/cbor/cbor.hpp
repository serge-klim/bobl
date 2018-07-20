// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <cstdint>
#include <type_traits>

namespace bobl{ namespace cbor {

enum Type : std::uint8_t
{
	UnsignedInt = (0 << 5),
	UnsignedInt8 = UnsignedInt | 24,
	UnsignedInt16 = UnsignedInt | 25,
	UnsignedInt32 = UnsignedInt | 26,
	UnsignedInt64 = UnsignedInt | 27,
	NegativeInt = (1 << 5),
	NegativeInt8 = NegativeInt | 24,
	NegativeInt16 = NegativeInt | 25,
	NegativeInt32 = NegativeInt | 26,
	NegativeInt64 = NegativeInt | 27,
	ByteString = (2 << 5),
	TextString = (3 << 5),
	Array = (4 << 5),
	Dictionary = (5 << 5),
	Tag = (6 << 5),
	SimpleValue = (7 << 5),
	False = SimpleValue | 20,
	True = SimpleValue | 21,
	Null = SimpleValue | 22,
	Float16 = SimpleValue | 25,
	Float32 = SimpleValue | 26,
	Float64 = SimpleValue | 27,
	Break = SimpleValue | 31
};

enum Tag : std::uint8_t
{
	DataTimeString = 0,
	DataTimeNumerical = 1,
	UUID = 37
};


inline const char* to_string(Type type);

}/*namespace bson*/ } /*namespace cbor*/