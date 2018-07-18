// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/nvariant.hpp"
#include "bobl/utility/diversion.hpp"
#include "bobl/bobl.hpp"
#include <boost/format.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/mpl/begin_end.hpp>	
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/range_c.hpp>
#include <utility>
#include <type_traits>

namespace bobl{ namespace utility{

namespace details {

template<typename T, std::size_t Position>
struct FusedStructName
{
	constexpr char const* operator()() const { return boost::fusion::extension::struct_member_name<T, Position>::call(); }
};


template<typename T, typename MemberType, std::size_t Position>
struct PositionAsName
{
	char const* operator()() const
	{ 
		static auto name = str(boost::format("_%1%")%Position);
		return name.c_str();
	}
};

} /*namespace details*/

template<typename T, typename MemberType, std::size_t Position, typename Options>
class DefaultMemberName
{
	struct NoName {};

	struct VariantUseTypeName { constexpr char const* operator()() const { return "_"; } };

	using CheckUseTypeName = typename std::conditional<bobl::utility::VariantUseTypeName<MemberType, Options>::value, VariantUseTypeName, NoName>::type;

	using CheckOptions = typename std::conditional<bobl::utility::options::Contains<Options, bobl::options::UsePositionAsName>::value,
															details::PositionAsName<T, MemberType, Position>,
															CheckUseTypeName>::type;
public:
	//1. std::is_same<typename boost::fusion::traits::tag_of<T>::type, boost::fusion::struct_tag>::value
	//2. bobl::Option::UsePositionAsName
	//3. diversion::variant<bobl::UseTypeName, Types...>
	using type = typename std::conditional<std::is_same<typename boost::fusion::traits::tag_of<T>::type, boost::fusion::struct_tag>::value,
								details::FusedStructName<T, Position>,
								CheckOptions>::type;
};

} //namespace utility

template<typename Type, typename MemberType, std::size_t Position, typename Options>
class MemberName : public utility::DefaultMemberName <Type, MemberType, Position, Options>::type {};


namespace utility {

struct ObjectNameIrrelevant
{
    static constexpr char const* name() { return ""; }
    template<typename T>
    constexpr bool compare(T const&) const noexcept { return true; }
};


template<typename T, typename Overloaded = std::true_type>
struct GetNameType
{
	using type = ObjectNameIrrelevant;
};

template<typename T>
struct GetNameType<T, typename std::is_constructible<std::string, decltype(T{}/*std::declval<T>()*/())>::type >
{
	struct TypeName
	{
		static /*constexpr*/ auto name() -> decltype(T{}/*std::declval<T>()*/()) { return T{}(); }
		bool compare(diversion::string_view n) const { return n.compare(name()) == 0; }
		bool compare(std::string const& n) const { return n.compare(name()) == 0; }
	};
	using type = TypeName;
};

namespace details {

template<typename Sequence, typename Options>
class NamedSequence
{
	static_assert(boost::fusion::traits::is_sequence<Sequence>::value, "bobl::utility::DictionaryDecoder expects Sequence to be a boost::fusion sequence");
	using Range = boost::mpl::range_c<std::size_t, 0, boost::fusion::result_of::size<Sequence>::value>;

	template<typename Position>
	using MemName = bobl::utility::GetNameType<bobl::MemberName<Sequence, 
																		typename boost::fusion::result_of::value_at<Sequence, Position>::type,
																		Position::value, Options>>;
	template<typename Position>
	struct MemHasNoName
		: std::is_same<typename MemName<Position>::type, bobl::utility::ObjectNameIrrelevant> {};
public:
	using type = std::is_same<typename boost::mpl::find_if<Range, MemHasNoName<boost::mpl::placeholders::_1>>::type, typename boost::mpl::end<Range>::type>;
};

} // namespace details

template<typename Sequence, typename Options>
struct NamedSequence : details::NamedSequence<Sequence, Options>::type {};


}/*namespace utility*/} /*namespace bobl*/



