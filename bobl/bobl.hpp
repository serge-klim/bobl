// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <bobl/utility/flyweight.hpp>
#include <bobl/utility/any.hpp>
#include <stdexcept>
	
namespace bobl{ 

//inline namespace v1{

enum class Type
{
	Null,
	Bool,
	Tiny,
	UnsignedInt8,
	UnsignedInt16,
	UnsignedInt32,
	UnsignedInt64,
	Int8,
	Int16,
	Int32,
	Int64,
	Float8,
	Float16,
	Float32,
	Float64,
	String,
	Array,
	Dictionary,
	Timepoint,
	Binary,
	UUID,
	Integer = Int64,
	Double = Float64,
	Object = Dictionary
};

using exception = std::runtime_error;
using InvalidObject = exception;
using IncorrectObjectName = exception;
using IncorrectObjectType = exception;
using TypeNotSupported = exception;

using Overflow = std::overflow_error;
using InputToShort = std::length_error;
using RangeError = std::range_error;

namespace flyweight { 

namespace lite {

using utility::Any;
using utility::Object;
using utility::Array;

}// namespace lite

template<typename T>
class NameValue
{
public:
	NameValue(std::string&& name, T&& value) :name_{ std::move(name) }, value_{ std::move(value) } {}
	NameValue(std::string const& name, T value) :name_{ name }, value_{ std::move(value) } {}
	std::string const& name() const { return name_; }
	T const& value() const { return value_; }
private:
	std::string name_;
	T value_;
};

}/*namespace flyweight*/


struct Versioned {};
struct UseTypeName {};

inline const char* to_string(Type type)
{
	const char* res = "unknown type";
	switch (type)
	{
		case Type::Null:
			res = "Null";
			break;
		case Type::Bool:
			res = "Bool";
			break;
		case Type::Tiny:
			res = "Tiny";
			break;
		case Type::UnsignedInt8:
			res = "UnsignedInt32";
			break;
		case Type::UnsignedInt16:
			res = "UnsignedInt16";
			break;
		case Type::UnsignedInt32:
			res = "UnsignedInt32";
			break;
		case Type::UnsignedInt64:
			res = "UnsignedInt64";
			break;
		case Type::Int8:
			res = "Int32";
			break;
		case Type::Int16:
			res = "Int16";
			break;
		case Type::Int32:
			res = "Int32";
			break;
		case Type::Int64:
			res = "Int64";
			break;
		case Type::Float8:
			res = "Float8";
			break;
		case Type::Float16:
			res = "Float16";
			break;
		case Type::Float32:
			res = "Float32";
			break;
		case Type::Float64:
			res = "Float64";
			break;
		case Type::String:
			res = "String";
			break;
		case Type::Array:
			res = "Array";
			break;
		case Type::Dictionary:
			res = "Dictionary";
			break;
		case Type::Timepoint:
			res = "Timepoint";
			break;
		case Type::Binary:
			res = "Binary";
			break;
		case Type::UUID:
			res = "UUID";
			break;
	}
	return res;
}

namespace utility {
// alows to initilize provided type
template<typename Sequence, typename T>
class DefaultValue{ public: void operator()(T&) const {} };
} // namespace utility

//}/*inline namespace v1*/ 

} /*namespace bobl*/

