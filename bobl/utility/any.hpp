// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/diversion.hpp"
#include <boost/range/iterator_range.hpp>
#include <vector>
#include <cstdint>

namespace bobl{ namespace flyweight { 
	
namespace utility{

enum class AnyTag
{
	Any,
	Array,
	Object
};

template <AnyTag Tag>
class AnyType
{
public:
	template<typename Iterator>
	AnyType(Iterator begin, Iterator end);
	AnyType(std::string::const_iterator begin, std::string::const_iterator end);
	AnyType(std::wstring::const_iterator begin, std::wstring::const_iterator end);
private:
	diversion::variant<
		boost::iterator_range<std::uint8_t const*>,
		boost::iterator_range<std::string::const_iterator>,
		boost::iterator_range<std::wstring::const_iterator>,
		std::vector<std::uint8_t>
	> range_;
};

using Any = AnyType<AnyTag::Any>;
using Array = AnyType<AnyTag::Array>;
using Object = AnyType<AnyTag::Object>;

template<typename T> struct IsAnyType : std::false_type {};
template<AnyTag Tag> struct IsAnyType<AnyType<Tag>> : std::true_type {};

//template<typename T>
//auto cbegin(T const& any) -> typename std::enable_if<IsAnyType<T>::value, typename T::iterator>::type { return any.range_.begin(); }
//template<typename T>
//auto cend(T const& any) -> typename std::enable_if<IsAnyType<T>::value, typename T::iterator>::type { return any.range_.end(); }

}/*namespace utility*/

namespace lite { namespace utility{

template<typename Iterator, bobl::flyweight::utility::AnyTag Tag> class AnyType;
namespace details {

template<typename Iterator, bobl::flyweight::utility::AnyTag Tag> Iterator begin_raw(bobl::flyweight::lite::utility::AnyType<Iterator, Tag> const&);
template<typename Iterator, bobl::flyweight::utility::AnyTag Tag> Iterator end_raw(bobl::flyweight::lite::utility::AnyType<Iterator, Tag> const&);

}//namespace utility

template<typename Iterator, bobl::flyweight::utility::AnyTag Tag>
class AnyType
{
	template<typename I, bobl::flyweight::utility::AnyTag T> I friend details::begin_raw(AnyType<I, T> const&);
	template<typename I, bobl::flyweight::utility::AnyTag T> I friend details::end_raw(AnyType<I, T> const&);
public:
	using iterator = Iterator;
	AnyType(boost::iterator_range<iterator>&& range) : range_{ std::move(range) } {}
	AnyType(Iterator begin, Iterator end) : range_{ std::move(begin), std::move(end) } {}
private:
	boost::iterator_range<Iterator> range_;
};

template<typename Iterator>
using Any = AnyType<Iterator, bobl::flyweight::utility::AnyTag::Any>;

template<typename Iterator>
using Array = AnyType<Iterator, bobl::flyweight::utility::AnyTag::Array>;

template<typename Iterator>
using Object = AnyType<Iterator, bobl::flyweight::utility::AnyTag::Object>;

template<typename T> struct IsAnyType : std::false_type {};
template<typename Iterator, bobl::flyweight::utility::AnyTag Tag> struct IsAnyType<AnyType<Iterator, Tag>> : std::true_type {};

template<typename T> struct IsAny : std::false_type {};
template<typename Iterator> struct IsAny<AnyType<Iterator, bobl::flyweight::utility::AnyTag::Any>> : std::true_type {};

template<typename T> struct IsArray : std::false_type {};
template<typename Iterator> struct IsArray<AnyType<Iterator, bobl::flyweight::utility::AnyTag::Array>> : std::true_type {};

template<typename T> struct IsObject : std::false_type {};
template<typename Iterator> struct IsObject<AnyType<Iterator, bobl::flyweight::utility::AnyTag::Object>> : std::true_type {};

namespace details {

template<typename Iterator, bobl::flyweight::utility::AnyTag Tag> Iterator begin_raw(bobl::flyweight::lite::utility::AnyType<Iterator, Tag> const& any) { return any.range_.begin(); }
template<typename Iterator, bobl::flyweight::utility::AnyTag Tag> Iterator end_raw(bobl::flyweight::lite::utility::AnyType<Iterator, Tag> const& any) { return any.range_.end(); }

}//namespace details

} /*namespace lite*/ }/*namespace utility*/

} /*namespace flyweight*/ } /*namespace bobl*/


