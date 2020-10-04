// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include "bobl/bobl.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <iterator>
#include <type_traits>

namespace bobl{ namespace utility{

template<typename T>
T read(T const*& begin, T const* end)
{
	if(begin == end)
		throw bobl::InputToShort{"input to short"};
	auto res = *begin;
	++begin;
	return res;
}

template<typename T, typename Iterator>
auto read(Iterator& begin, Iterator end) -> typename std::enable_if<std::is_pointer<Iterator>::value, T>::type
{
	if (std::size_t(std::distance(begin, end)) < sizeof(T))
		throw bobl::InputToShort{ "input to short" };
	auto res = *reinterpret_cast<T const*>(begin);
	std::advance(begin, sizeof(T));
	return res;
}

template<typename T, typename Iterator>
auto read(Iterator& begin, Iterator end) -> typename std::enable_if<!std::is_pointer<Iterator>::value, T>::type
{
	T res;
	for (std::size_t i = 0; i < sizeof(res); ++i)
	{
		if (begin == end)
			throw bobl::InputToShort{ "input to short" };
		//res = res << 8;
		res |= T(*begin++);
	}
}


template<typename T, typename Options, typename Decoder, typename RawIterator = typename Decoder::iterator>
class Iterator : public boost::iterator_facade<Iterator<T, Options, Decoder, RawIterator>, T, boost::forward_traversal_tag, T>
{
	template<typename U, typename OtherOptions, typename OtherDecoder, typename OtherRawIterator>
	friend class Iterator;
public:
	Iterator(RawIterator begin, RawIterator end) : current_{ begin }, next_{ begin }, end_{ end } {}
	Iterator(boost::iterator_range<RawIterator> range) : current_{ range.begin() }, next_{ range.begin() }, end_{ range.end() } {}
	Iterator(Iterator&&) = default;
	Iterator& operator= (Iterator&&) = default;
	Iterator(Iterator const&) = default;
	Iterator& operator= (Iterator const&) = default;
	template<typename U, typename OtherOptions>
	Iterator(Iterator<U, OtherOptions, Decoder, RawIterator> const& other) : current_{ other.current_ }, next_{ other.next_ }, end_{ other.end_ } {}
	template<typename U, typename OtherOptions>
	Iterator(Iterator<U, OtherOptions, Decoder, RawIterator>&& other) : current_{ std::move(other.current_) }, next_{ std::move(other.next_) }, end_{ std::move(other.end_) } {}
	template<typename U, typename OtherOptions>
	Iterator& operator=(Iterator<U, OtherOptions, Decoder, RawIterator> const& other)
	{
		current_ = other.current_;
		next_ = other.next_; 
		end_ = other.end_;
		return *this;
	}

	template<typename U, typename OtherOptions>
	Iterator& operator= (Iterator<U, OtherOptions, Decoder, RawIterator>&& other)
	{
		current_ = std::move(other.current_);
		next_ = std::move(other.next_);
		end_ = std::move(other.end_);
		return *this;
	}

	void increment() 
	{ 
		if (next_ == current_)
			Decoder::template decode<bobl::flyweight::lite::Any<RawIterator>, Options>(next_, end_);
		current_ = next_;
	}
	T dereference() const 
	{ 
		next_ = current_;
		return Decoder::template decode<T, Options>(next_, end_);
	}
	template<typename U, typename OtherOptions>
	bool equal(Iterator<U, OtherOptions, Decoder, RawIterator> const& other) const { return current_ == other.current_ && end_ == other.end_;}
private:
	RawIterator current_;
	mutable RawIterator next_;
	RawIterator end_;
};

}/*namespace utility*/} /*namespace bobl*/

