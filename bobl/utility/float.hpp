// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <boost/cstdfloat.hpp>
#include <type_traits>
#include <cstdint>

namespace bobl{ namespace utility {

template<std::size_t>
class FloatConverter{};

template<>
class FloatConverter<sizeof(std::uint16_t)>
{
	using Float = float;
public:
	template<typename T>
	auto operator()(T i) -> typename std::enable_if<std::is_integral<T>::value, Float>::type
	{
		int exp = (i >> 10) & 0x1f;
		int mant = i & 0x3ff;
		double val;
		if (exp == 0)
			val = std::ldexp(mant, -24);
		else
		{
			if (exp != 31)
				val = std::ldexp(mant + 1024, exp - 25);
			else
				val = mant == 0 ? INFINITY : NAN;
		}
		return Float(i & 0x8000 ? -val : val);
	}

	template<typename T>
	auto operator()(T f) -> typename std::enable_if<std::is_floating_point<T>::value, std::uint16_t>::type;
};

template<typename Float, typename Int>
struct FloatConverterBase
{
	union Union
	{
		Int i;
		Float f;
	};

	template<typename T>
	auto operator()(T i) -> typename std::enable_if<std::is_integral<T>::value, Float>::type
	{
		Union u;
		u.i = Int(i);
		return u.f;
	}
	template<typename T>
	auto operator()(T f) -> typename std::enable_if<std::is_floating_point<T>::value, Int>::type
	{
		Union u;
		u.f = Float(f);
		return u.i;
	}
};

template<>
class FloatConverter<sizeof(std::uint32_t)> : public FloatConverterBase<boost::float32_t, std::uint32_t>{};

template<>
class FloatConverter<sizeof(std::uint64_t)> : public FloatConverterBase<boost::float64_t, std::uint64_t> {};


}/*namespace utility*/ } /*namespace bobl*/

