// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/details/header.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/utils.hpp"
#include "bobl/bobl.hpp"
#include <boost/endian/conversion.hpp>
#include <boost/format.hpp>
#include <string>
#include <type_traits>


namespace bobl{ namespace bson { namespace flyweight{ namespace details{ 


template<typename T, typename Options>
class RelaxedIntegerHandler : public ValueHandlerBase
{
	using ValueHandlerBase::ValueHandlerBase;
public:
	T operator()() const 
	{
		auto val = value();
		return T(type() == bobl::bson::Int32 ? cast_pointer<T>(reinterpret_cast<std::int32_t const*> (val)) : cast_pointer<T>(reinterpret_cast<std::int64_t const*> (val)));
	}

	static RelaxedIntegerHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		if ( !is(header) )
			throw bobl::IncorrectObjectType{ str(boost::format("BSON object %1% has unexpected type : %2% (%3$#x) insted of expected Int32 or Int64") % header.name() % to_string(header.type()) % int(header.type()) ) };
		begin = header.validate(end);
		return { std::move(header) };
	}

	static bool is(details::ObjectHeader const& header) 
	{ 
		auto type = header.type();
		return type == bobl::bson::Int32 || type == bobl::bson::Int64;
	}
private:
	template<typename U, typename X>
	static auto cast_pointer(X const* ptr) -> typename std::enable_if<(sizeof(U) < sizeof(X)), U>::type
	{
		auto raw = boost::endian::little_to_native(*ptr);
		auto res = U(raw);
		if (X(res) != raw)
			throw RangeError{ str(boost::format("\"%1%\" can't hold parsed integer value \"%2%\"") % boost::typeindex::type_id<U>().pretty_name() % raw) };
		return res;
	}
	template<typename U, typename X>
	static auto cast_pointer(X const* ptr) -> typename std::enable_if<(sizeof(U) >= sizeof(X)), U>::type
	{
		return U(boost::endian::little_to_native(*ptr));
	}
};

template<typename T, typename Options>
class StrictIntegerHandler : public ValueHandlerBase
{
	using ValueType = typename std::conditional<(sizeof(T) > sizeof(std::int32_t)), std::int64_t, std::int32_t>::type;
	using BsonType = typename std::conditional<(sizeof(T) > sizeof(std::int32_t)), std::integral_constant<bobl::bson::Type, bobl::bson::Int64>, std::integral_constant<bobl::bson::Type, bobl::bson::Int32>>::type;
	using ValueHandlerBase::ValueHandlerBase;
public:
	T operator()() const { return T(boost::endian::little_to_native(*reinterpret_cast<ValueType const*> (value()))); }

	static StrictIntegerHandler decode(ObjectHeader&& header, bobl::bson::flyweight::Iterator& begin, bobl::bson::flyweight::Iterator end)
	{
		begin = header.validate<BsonType::value>(end);
		return { std::move(header) };
	}
	static bool is(details::ObjectHeader const& header) { return header.type() == BsonType::value; }
};


} /*namespace details*/ } /*namespace flyweight*/ } /*namespace bson*/ } /*namespace bobl*/

