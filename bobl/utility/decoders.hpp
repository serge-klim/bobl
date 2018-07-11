// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/utility/names.hpp"
#include "bobl/names.hpp"
#include "bobl/options.hpp"
#include "bobl/bobl.hpp"
#include "bobl/utility/type_name.hpp"
#include "bobl/utility/diversion.hpp"
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/and.hpp>
#include <boost/format.hpp>
#include <bitset>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <type_traits>

namespace bobl{ namespace utility{

template<typename Sequence, typename Options>
using DictionaryDecoderCompatible = boost::mpl::and_<std::is_default_constructible<Sequence>, boost::fusion::traits::is_sequence<Sequence>, NamedSequence<Sequence, Options>>;

template<typename Sequence, typename ObjectDecoder, typename ...Options>
class DictionaryDecoder
{
	using Key = typename ObjectDecoder::Name;
	using Skipper = typename ObjectDecoder::Skipper;

	struct Emplacer
	{
	public:
		template<std::size_t N, typename Iterator>
		auto emplace(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Key&& key, Iterator& begin, Iterator end)  -> typename std::enable_if<N != boost::fusion::result_of::size<Sequence>::value>::type;
		template<std::size_t N, typename Iterator>
		auto emplace(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Key&& key, Iterator& begin, Iterator end)  -> typename std::enable_if<N == boost::fusion::result_of::size<Sequence>::value>::type;
		Sequence value() && ;
	private:
		template<std::size_t N>
		auto initialize() -> typename std::enable_if<N != boost::fusion::result_of::size<Sequence>::value>::type;
		template<std::size_t N>
		auto initialize() -> typename std::enable_if<N == boost::fusion::result_of::size<Sequence>::value>::type {}
	private:
		std::bitset<boost::fusion::result_of::size<Sequence>::value> initialized_;
		Sequence sequence_/* = {}*/;
	};

public:
	static_assert(DictionaryDecoderCompatible<Sequence, bobl::Options<Options...>>::value, "bobl::utility::DictionaryDecoder expects Sequence to be copy constructible," 
																	"a boost::fusion sequence with named members");

	template<typename Iterator, typename EndOfObject = std::equal_to<Iterator>>
	Sequence operator()(Iterator& begin, Iterator end, EndOfObject eoo = EndOfObject{}) const;
	template<typename Iterator, typename EndOfObject = std::equal_to<Iterator>>
	Sequence operator()(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Iterator& begin, Iterator end, EndOfObject eoo = EndOfObject{}) const;

	static auto name(Key const& key) -> decltype(ObjectDecoder::name(key)) { return ObjectDecoder::name(key); }
};

template<typename Sequence, typename ObjectDecoder, typename ...Options>
class Decoder
{
	static_assert(boost::fusion::traits::is_sequence<Sequence>::value, "bobl::utility::Decoder expects T to be a boost::fusion sequence");

	template<typename T, std::size_t N>
	struct ObjectNameTag
	{
		static char const* name() { return boost::fusion::extension::struct_member_name<T, N>::call(); }
		bool compare(diversion::string_view n) const { return n.compare(name()) == 0; }
		bool compare(std::string const& n) const { return n.compare(name()) == 0; }
	};
public:
	template<typename Iterator>
	Sequence operator()(Iterator& begin, Iterator end) const;
	template<typename Iterator>
	Sequence operator()(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Iterator& begin, Iterator end) const { return decode(decoder, begin, end); }
private:
	template<typename Iterator, typename ...Args>
	typename std::enable_if<sizeof...(Args) != boost::fusion::result_of::size<Sequence>::value, Sequence>::type decode(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Iterator& begin, Iterator end, Args&&... args) const;

	template<typename Iterator, typename ...Args>
	typename std::enable_if<sizeof...(Args) == boost::fusion::result_of::size<Sequence>::value, Sequence>::type decode(typename ObjectDecoder::template rebase<Iterator, Options...>&, Iterator& /*begin*/, Iterator /*end*/, Args&&... args) const
	{
		return Sequence{ std::forward<Args>(args)... };
	}
};


}/*namespace bobl*/} /*namespace utility*/


template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<typename Iterator, typename EndOfObject /*= std::equal_to<Iterator>*/>
Sequence bobl::utility::DictionaryDecoder<Sequence, ObjectDecoder, Options...>::operator()(Iterator& begin, Iterator end, EndOfObject eoo /*= EndOfObject{}*/) const
{ 
	typename ObjectDecoder::template rebase<Iterator, Options...> decoder;
	return operator()(decoder, begin, end, std::move(eoo)); 
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<typename Iterator, typename EndOfObject /*= std::equal_to<Iterator>*/>
Sequence bobl::utility::DictionaryDecoder<Sequence, ObjectDecoder, Options...>::operator()(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Iterator& begin, Iterator end, EndOfObject eoo /*= EndOfObject{}*/) const
{
	Emplacer emplacer;
	while (!eoo(begin, end))
	{
		auto key = decoder.decode_name(begin, end);
		emplacer.template emplace<0>(decoder, std::move(key), begin, end);
	}
	return std::move(emplacer).value();
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
Sequence bobl::utility::DictionaryDecoder<Sequence, ObjectDecoder, Options...>::Emplacer::value() &&
{
	if (!initialized_.all())
	{
		if /*constexpr*/ (bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::ExacMatch>::value)
			throw bobl::InputToShort{ str(boost::format("not enough data to completely initialize dictionary %1%>") % bobl::utility::type_name<Sequence>()) };
		initialize<0>();
	}
	return std::move(sequence_);
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<std::size_t N>
auto bobl::utility::DictionaryDecoder<Sequence, ObjectDecoder, Options...>::Emplacer::initialize() -> typename std::enable_if<N != boost::fusion::result_of::size<Sequence>::value>::type
{
	if (!initialized_[N])
	{
		using Type = typename boost::fusion::result_of::value_at_c<Sequence, N>::type;
		bobl::utility::DefaultValue<Sequence, Type>{}(boost::fusion::at_c<N>(sequence_));
		initialized_.set(N);
	}
	if (!initialized_.all())
		initialize<N + 1>();
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<std::size_t N, typename Iterator>
auto bobl::utility::DictionaryDecoder<Sequence, ObjectDecoder, Options...>::Emplacer::emplace(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Key&& key, Iterator& begin, Iterator end)  
		-> typename std::enable_if<N != boost::fusion::result_of::size<Sequence>::value>::type
{
	using Type = typename boost::fusion::result_of::value_at_c<Sequence, N>::type;
	using MemberName = typename bobl::utility::GetNameType<bobl::MemberName<Sequence, Type, N, bobl::Options<Options...>>>::type;
	static_assert(!std::is_same<MemberName, bobl::utility::ObjectNameIrrelevant>::value, "seems like this member has no name attached");
	if (MemberName{}.compare(name(key)))
	{
		if (initialized_.test(N))
			throw bobl::InvalidObject(str(boost::format("more then one key \"%1%\" found in the dictionary") % name(key)));
		boost::fusion::at_c<N>(sequence_) = decoder.template decode<Type>(std::move(key), begin, end);
		initialized_.set(N);
	}
	else
		emplace<N + 1>(decoder, std::move(key), begin, end);
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<std::size_t N, typename Iterator>
auto bobl::utility::DictionaryDecoder<Sequence, ObjectDecoder, Options...>::Emplacer::emplace(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Key&& key, Iterator& begin, Iterator end)  
			-> typename std::enable_if<N == boost::fusion::result_of::size<Sequence>::value>::type
{
	if /*constexpr*/ (bobl::utility::options::Contains<bobl::Options<Options...>, bobl::options::ExacMatch>::value)
		throw bobl::InvalidObject(str(boost::format("unexpected key \"%1%\" found in the dictionary") % name(key)));

	decoder.template decode<Skipper>(std::move(key), begin, end);
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<typename Iterator>
Sequence bobl::utility::Decoder<Sequence, ObjectDecoder, Options...>::operator()(Iterator& begin, Iterator end) const 
{ 
	typename ObjectDecoder::template rebase<Iterator, Options...> decoder;
	return operator()(decoder, begin, end); 
}

template<typename Sequence, typename ObjectDecoder, typename ...Options>
template<typename Iterator, typename ...Args>
typename std::enable_if<sizeof...(Args) != boost::fusion::result_of::size<Sequence>::value, Sequence>::type 
	bobl::utility::Decoder<Sequence, ObjectDecoder, Options...>::decode(typename ObjectDecoder::template rebase<Iterator, Options...>& decoder, Iterator& begin, Iterator end, Args&&... args) const
{
	using Type = typename boost::fusion::result_of::value_at_c<Sequence, sizeof...(Args)>::type;
    using NameType = typename std::conditional<
                                    std::is_same<typename boost::fusion::traits::tag_of<Sequence>::type, boost::fusion::struct_tag>::value
                                    , ObjectNameTag<Sequence,sizeof...(Args)>
                                    , ObjectNameIrrelevant>::type;

	auto value = decoder.template decode<Type, sizeof...(Args)>(begin, end, NameType{});
	return decode(decoder, begin, std::move(end), std::forward<Args>(args)..., std::move(value));
}
