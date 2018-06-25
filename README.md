### bobl

[![Build Status](https://travis-ci.com/serge-klim/bobl.svg?branch=master)](https://travis-ci.com/serge-klim/bobl.svg?branch=master)
[![Build status](https://ci.appveyor.com/api/projects/status/pj023kupejhaccr7?svg=true)](https://ci.appveyor.com/project/serge-klim/bobl)
[![codecov](https://codecov.io/gh/serge-klim/bobl/branch/master/graph/badge.svg)](https://codecov.io/gh/serge-klim/bobl)


It is compile time coder/decoder generator, at the moment it supports basic [bson](http://bsonspec.org/spec.html) and [cbor](http://cbor.io/spec.html) encoding/decoding.

### How it works:

lets say that there is encoded bson object:
```
	//{'enabled': True, 'id': 100, 'name': 'the name', 'theEnum': 2}
	//(55) : b'7\x00\x00\x00\x08enabled\x00\x01\x10id\x00d\x00\x00\x00\x02name\x00\t\x00\x00\x00the name\x00\x10theEnum\x00\x02\x00\x00\x00\x00'
```
    it can be decoded to std::tuple like this:
```
    	std::tuple<bool, int, std::string, int> res = bobl::bson::decode<std::tuple<bool, int, std::string, int>>(begin, end);
```
    or std::tuple in decode can be omited:
```
    	std::tuple<bool, int, std::string, int> res = bobl::bson::decode<bool, int, std::string, int>(begin, end);
```
BSON complete example: [simple1.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/simple1.cpp)    
CBOR complete example: [simple1.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/simple1.cpp)
```
    #include "bobl/bson/decode.hpp"

	std::uint8_t data[] = {
		0x37, 0x0,  0x0,  0x0,  0x8,  0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65,
		0x64, 0x0,  0x1,  0x10, 0x69, 0x64, 0x0,  0x64, 0x0,  0x0,  0x0,
		0x2,  0x6e, 0x61, 0x6d, 0x65, 0x0,  0x9,  0x0,  0x0,  0x0,  0x74,
		0x68, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x0,  0x10, 0x74, 0x68,
		0x65, 0x45, 0x6e, 0x75, 0x6d, 0x0,  0x2,  0x0,  0x0,  0x0,  0x0 };

	uint8_t const* begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	auto res = bobl::bson::decode<bool, int, std::string, TheEnumClass>(begin, end);
```
it supports [boost::fusion](https://www.boost.org/doc/libs/1_67_0/libs/fusion/doc/html/) sequences
so above could be decoded like this:
```
enum class TheEnumClass { None, One, Two, Three };

struct Simple
{
  bool enabled;
  int id;
  std::string name;
  TheEnumClass theEnum;
};

BOOST_FUSION_ADAPT_STRUCT(
  Simple,
  enabled,
  id,
  name,
  theEnum)

Simple simple = bobl::bson::decode<Simple>(begin, end);
```
as well as encoded:

```
std::vector<std::uint8_t> blob = bobl::bson::encode<Simple>(begin, end);
```
BSON complete example: [simple3.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/simple3.cpp)  
CBOR complete example: [simple3.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/simple3.cpp)


tuple also can be encoded, but BSON requires names for objects. In case of fused structure, member name becames appropriate object name with tuples some naming needed, to solve it there is few options:
1. position of tuple element could became an element name:
```
    auto value = std::make_tuple(true, 100, std::string{ "the name" }, TheEnumClass::Two);
	auto data = bobl::bson::encode<bobl::options::UsePositionAsName>(value);
```
The resulting object will look like this (pseudo-json representation):
```
	{'_0': True, '_1': 100, '_2': 'the name', '_3': 2}
``` 
2. another way to do it is specialize:
```
#include "bobl/names.hpp"

namespace bobl {
  template<typename Type, typename MemberType, std::size_t Position, typename Options> class MemberName;
}
``` 
something like this:
```
namespace bobl{

	template<typename MemberType, typename Options> class MemberName <SimpleTuple, MemberType, 0, Options>
	{
	public:
		constexpr char const* operator()() const { return "enabled"; }
	};

	template<typename Options> class MemberName <SimpleTuple, int, 1, Options>
	{
	public:
		constexpr char const* operator()() const { return "id"; }
	};

	template<std::size_t Position, typename Options> class MemberName <SimpleTuple, std::string, Position , Options>
	{
	public:
		constexpr char const* operator()() const { return "name"; }
	};

	template<typename MemberType, typename Options> class MemberName <SimpleTuple, MemberType, 3, Options>
	{
	public:
		constexpr char const* operator()() const { return "enm"; }
	};

}//namespace bobl

``` 
so encode on tuple would work the same way as it does on fused structures:

``` 
  auto tuple = std::make_tuple(true, 100, std::string{ "the name" }, Enum::Two);
  auto data = bobl::bson::encode(tuple);
```
BSON complete example: [named_tuple.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/named_tuple.cpp)  
CBOR complete example: [named_tuple.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/named_tuple.cpp)


the library could handle more complicated  types, as such:

```
struct Extended
{
	int id;
	Simple simple;
	std::vector<int> ints;
	std::vector<Simple> simples;
	boost::variant<int,Simple, std::string, std::vector<Simple>> var;
	boost::uuids::uuid uuid;
	boost::optional<Enum> enm;
	std::vector<std::uint8_t> binary; // this will be encoded as binary object
};

BOOST_FUSION_ADAPT_STRUCT(
	Extended,
	id,
	simple,
	ints,
	simples,
	var,
	uuid,
	enm,
	binary)

    auto extended = Extended{};
	std::vector<std::uint8_t> encoded =  bobl::bson::encode<Extended>(extended)
```
BSON complete example: [extended.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/extended.cpp)  
CBOR complete example: [extended.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/extended.cpp)


### Requirements
 - c++11 suported compiler (clang 3.6+, gcc 4.8.5+, msvc-14.1+ )
 - [boost](http://www.boost.org) ([MPL](http://www.boost.org/doc/libs/1_66_0/libs/mpl/doc/index.html), [Fusion](http://www.boost.org/doc/libs/1_66_0/libs/fusion/doc/html/), [Uuid](https://www.boost.org/doc/libs/1_67_0/libs/uuid/doc/index.html), [Endian](https://www.boost.org/doc/libs/1_67_0/libs/endian/doc/index.html), [Range](https://www.boost.org/doc/libs/1_67_0/libs/range/doc/html/index.html), [Format](https://www.boost.org/doc/libs/1_67_0/libs/format/))

