// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <cstdint>
#include <type_traits>

namespace bobl{ namespace cbor {

enum Type : std::uint8_t
{
	UnsignedInt = (0 << 5),
	NegativeInt = (1 << 5),
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