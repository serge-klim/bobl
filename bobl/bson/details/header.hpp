// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bson/bson.hpp"
//#include "bobl/utility/names.hpp"
////#include "bobl/utility/iterator.hpp"
#include "bobl/bobl.hpp"
#include <boost/format.hpp>
#include "bobl/utility/diversion.hpp"
#include <boost/endian/conversion.hpp>
//#include <iterator>
#include <algorithm>
#include <cstdint>
#include <cassert>

namespace bobl{ namespace bson { namespace flyweight{ 

using Iterator = std::uint8_t const*;

namespace details{ 

class ObjectHeader
{
public:
	ObjectHeader(flyweight::Iterator begin, flyweight::Iterator end) : type_{begin}
	{
		if(begin == end || (begin = std::find(++begin, end, 0)) == end)
			throw bobl::InvalidObject{ "not enough data provided to construct BSON object" };
		value_ = ++begin;
	}

	bson::Type type() const { return bson::Type(*type_); }

	diversion::string_view name() const 
	{ 
		assert(std::distance(type_, value_) >= 2);  
		return diversion::string_view{ reinterpret_cast<char const*>(type_ + 1), std::size_t(std::distance(type_, value_) - 2) }; 
	}

	flyweight::Iterator value() const { return value_; }

	flyweight::Iterator position() const { return reinterpret_cast<flyweight::Iterator>(type_);	}


	template<bobl::bson::Type Type>
	flyweight::Iterator validate(flyweight::Iterator end) const
	{
		if(type() != Type)
			throw bobl::IncorrectObjectType{ str(boost::format("BSON object %1% has unexpected type : %2% (%3$#x) insted of expected: %4%") % name() % to_string(type()) % int(type()) % to_string(Type)) };
		return validate_<typename bobl::bson::SizeOf<Type>::type>(end);
	}

	flyweight::Iterator validate(flyweight::Iterator end) const
	{
		auto distance = std::distance(value_, end);
		typename std::iterator_traits<decltype(value_)>::difference_type size = 0;
		switch (auto type = ObjectHeader::type())
		{
			case bobl::bson::Double:
				size = bobl::bson::SizeOf<bobl::bson::Double>::type::value;
				break;
			case bobl::bson::DBPointer:
				size = sizeof(std::uint8_t) * 11/*12*/; /*should be 12 but next case will add 1 - binary subtype*/
			case bobl::bson::Binary:
				size += sizeof(std::uint8_t); //subtype
			case bobl::bson::Utf8String:
			case bobl::bson::Symbol:
//			case bobl::bson::Binary:
				size += sizeof(std::uint32_t);
				if (size > distance)
					break;
			case bobl::bson::Array:
			case bobl::bson::EmbeddedDocument:
				size += boost::endian::little_to_native(*reinterpret_cast<std::uint32_t const*>(value_));
				break;
			case bobl::bson::Bool:
				size = bobl::bson::SizeOf<bobl::bson::Bool>::type::value;
				break;
			case bobl::bson::UTCDateTime:
				size = bobl::bson::SizeOf<bobl::bson::UTCDateTime>::type::value;
				break;
			case bobl::bson::Int32:
				size = bobl::bson::SizeOf<bobl::bson::Int32>::type::value;
				break;
			case bobl::bson::Timestamp:
				size = bobl::bson::SizeOf<bobl::bson::UTCDateTime>::type::value;
				break;
			case bobl::bson::Int64:
				size = bobl::bson::SizeOf<bobl::bson::Int64>::type::value;
				break;
			case bobl::bson::Null:
				size = bobl::bson::SizeOf<bobl::bson::Null>::type::value;
				break;
			case bobl::bson::MinKey:
				size = bobl::bson::SizeOf<bobl::bson::MinKey>::type::value;
				break;
			case bobl::bson::MaxKey:
				size = bobl::bson::SizeOf<bobl::bson::MaxKey>::type::value;
				break;
			default:
				throw bobl::TypeNotSupported{ str(boost::format("not supported BSON type : %1% (%2$#x)") % to_string(type) % int(type)) };
		}

		if (size > distance)
			throw bobl::InvalidObject{ str(boost::format("not enough data provided to construct BSON object \"%1%\" of type : %2% (%3$#x)") % name() % to_string(type()) % int(type())) };

		return value_ + size;
	}
private:
	template<typename SizeType>
	auto validate_(flyweight::Iterator end) const -> typename std::enable_if<bobl::bson::utility::FixedSize<SizeType>::value, flyweight::Iterator>::type
	{
		if (std::size_t(std::distance(value_, end)) < SizeType::value)
			throw bobl::InvalidObject{ str(boost::format("not enough data provided to construct BSON object \"%1%\" of type : %2% (%3$#x)") % name() % to_string(type()) % int(type())) };
		return value_ + SizeType::value;
	}

	template<typename SizeType>
	auto validate_(flyweight::Iterator end) const -> typename std::enable_if<!bobl::bson::utility::FixedSize<SizeType>::value, flyweight::Iterator>::type
	{
		return validate(end);
	}
private:
	flyweight::Iterator type_;
	flyweight::Iterator value_;
};

class ValueHandlerBase
{
protected:
	ValueHandlerBase(ObjectHeader&& header) : header_{ std::move(header) } {}
	flyweight::Iterator value() const { return header_.value(); }
	bson::Type type() const { return header_.type(); }
	flyweight::Iterator position() const { return header_.position(); }
public:
	diversion::string_view name() const { return header_.name(); }
private:
	ObjectHeader header_;
};

template<bobl::bson::Type Type>
class SimpleValueHandlerBase : public ValueHandlerBase
{
protected:
	static constexpr bobl::bson::Type BsonType = Type;
	using ValueHandlerBase::ValueHandlerBase;
	static SimpleValueHandlerBase decode(ObjectHeader&& header, flyweight::Iterator& begin, flyweight::Iterator end)
	{
		begin = header.validate<BsonType>(end);
		return { std::move(header) };
	}
public:
	static bool is(details::ObjectHeader const& header) { return header.type() == BsonType; }
};

} /*namespace details*/ } /*namespace flyweight*/ } /*namespace bson*/ } /*namespace bobl*/