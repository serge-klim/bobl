#include "bobl/bson/decode.hpp"
#include "bobl/bson/encode.hpp"
#include "bobl/bson/adapter.hpp"
#include <cassert>

class X
{
public:
	explicit X(int persistent) : persistent_{ persistent } {}
	int persistent() const { return persistent_;}
	int not_persistent() const { return notso_;}
	void not_persistent(int val) { notso_ = val;}
private:
	int persistent_;
	int notso_ = 0;
};

namespace bobl { namespace bson {
	template<>
	class Adapter<X, boost::mpl::true_> 
	{
	public:
		using type = int;
		X operator()(int x) const { return X{ x }; }
		int operator()(X const& x) const { return x.persistent(); }
	};
} /*namespace bson*/ } /*namespace bobl*/


struct Data
{
	X x;
};

BOOST_FUSION_ADAPT_STRUCT(Data, x)



int main()
{
	static_assert(bobl::utility::Adaptable<X, bobl::bson::Adapter<X>>::value, "seems bobl::utility::utility::decode::Adaptable doesn't work as expected");

	auto data = Data{ X { 101 } };
	std::vector<std::uint8_t> encoded =  bobl::bson::encode(data);
    auto begin = encoded.data();
	auto end = begin + encoded.size();
	auto decoded =  bobl::bson::decode<X>(begin, end);
	assert(decoded.persistent()  == 101);

	return 0;
}
