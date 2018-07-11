### bobl

[![Build Status](https://travis-ci.com/serge-klim/bobl.svg?branch=master)](https://travis-ci.com/serge-klim/bobl.svg?branch=master)
[![Build status](https://ci.appveyor.com/api/projects/status/pj023kupejhaccr7?svg=true)](https://ci.appveyor.com/project/serge-klim/bobl)
[![codecov](https://codecov.io/gh/serge-klim/bobl/branch/master/graph/badge.svg)](https://codecov.io/gh/serge-klim/bobl)


Compile time coder/decoder generator, at the moment it supports basic [bson](http://bsonspec.org/spec.html) and [cbor](http://cbor.io/spec.html) encoding/decoding.
Inspired by [Boost.Spirit](http://boost-spirit.com/home) and relays heavily on [Boost.Fusion](http://www.boost.org/doc/libs/1_66_0/libs/fusion/doc/html).

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
using tuples for complex types might be not really good idea that's where [Boost.Fusion](https://www.boost.org/doc/libs/1_50_0/libs/fusion/doc/html/fusion/adapted/adapt_struct.html) can be very useful, it allows addapt structure to heterogenous container. So above could be decoded like this:
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
std::vector<std::uint8_t> blob = bobl::bson::encode(simple);
```
BSON complete example: [simple3.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/simple3.cpp)  
CBOR complete example: [simple3.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/simple3.cpp)


std::tuple also can be encoded, but BSON requires names for objects. In case of adapted structures, member name became appropriate object name. With tuples some naming needed, to solve it there is few options:
1. position of tuple element can be used as an element name:
```
    auto value = std::make_tuple(true, 100, std::string{ "the name" }, TheEnumClass::Two);
	auto data = bobl::bson::encode<bobl::options::UsePositionAsName>(value);
```
The resulting object will look like this (pseudo-json representation):
```
	{'_0': True, '_1': 100, '_2': 'the name', '_3': 2}
``` 
2. another way to name tupple element is specialize MemberName class:
```
#include "bobl/names.hpp"

namespace bobl {
  template<typename Type, typename MemberType, std::size_t Position, typename Options> class MemberName;
}
``` 
like this:
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
so encode on tuple would work the same way as it does on [Boost.Fusion](https://www.boost.org/doc/libs/1_50_0/libs/fusion/doc/html/fusion/adapted/adapt_struct.html) adapted structures:

``` 
  auto tuple = std::make_tuple(true, 100, std::string{ "the name" }, Enum::Two);
  auto data = bobl::bson::encode(tuple);
```
BSON complete example: [named\_tuple.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/named_tuple.cpp)  
CBOR complete example: [named\_tuple.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/named_tuple.cpp)


the library could handle more compex types, for eg:

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
	std::chrono::system_clock::time_point tp;
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
	binary,
	tp)

    auto extended = Extended{};
	std::vector<std::uint8_t> encoded =  bobl::bson::encode(extended)
```
BSON complete example: [extended.cpp](https://github.com/serge-klim/bobl/blob/master/examples/bson/extended.cpp)  
CBOR complete example: [extended.cpp](https://github.com/serge-klim/bobl/blob/master/examples/cbor/extended.cpp)

#### Options.
Encoding/decoding can be controlled by options. At the moment following options are defined in [options.hpp](https://github.com/serge-klim/bobl/blob/master/bobl/options.hpp):
```
    struct RelaxedIntegers {};
    struct RelaxedFloats {};
    struct ExacMatch {};
    struct StructAsDictionary {}; 
    struct UsePositionAsName {};
    template<typename T> struct ByteType {};
    using IntegerOptimizeSize = RelaxedIntegers;
    template<typename T> struct HeterogeneousArray {};
    template<typename T> using NonUniformArray = HeterogeneousArray<T>;

```
This options can be used as explicitly set encode/decode functions template parameters and/or could be set per specific type by specializing bobl::EffectiveOptions structure:
```
	template<typename T, typename ...Options>
	struct EffectiveOptions
	{   
		using type = bobl::Options<Options...>;
	};
```
for example to encode tuple as object using tuples element position as an object member name following specialization of bobl::EffectiveOptions structure can be used:

```
namespace bobl{
	template<typename ...Types, typename ...Options>
	struct EffectiveOptions<std::tuple<Types...>, Options...>
	{   
		using type = bobl::Options<bobl::options::UsePositionAsName, Options...>;
	};
} //namespace bobl
```
Also if such options has to be set for specific  protocol(BSON or CBOR) bobl::<protocol name> namespace can be used, following will set bobl::options::UsePositionAsName for tuples used with cbor encode/decode functions:

```
namespace bobl{
  namespace cbor{
	template<typename ...Types, typename ...Options>
	struct EffectiveOptions<std::tuple<Types...>, Options...>
	{   
		using type = bobl::Options<bobl::options::UsePositionAsName, Options...>;
	};
  } //namespace cbor
} //namespace bobl
```

By default std::vector<std::uint8_t> encoded/decoded as byte string([CBOR](https://tools.ietf.org/html/rfc7049)  Major type 2) / binary data ([BSON](http://bsonspec.org/spec.html) - "\x05"). std::vector specialized with other types will be encoded as array type. bobl::option::ByteType allows to change this behavior. 

```
    std::vector<char> binary =  {100, 110, 120};
	bobl::cbor::encode<bobl::Options<bobl::options::ByteType<char>>>(...)
	// ...
    std::vector<char> = bobl::cbor::decode<std::vector<char>, bobl::Options<bobl::options::ByteType<char>>>(begin, end);

```
#### Decode objects encoded by other libraries.

By default encoded/decoded integer type based on its C++ type (not on its value size) which means that std::uint64_t containing value 1 will be encoded as [BSON](http://bsonspec.org/spec.html) - "\x12" (int64) type, it makes encoding/decoding bit faster. Using bobl::option::IntegerOptimizeSize option during encoding and bobl::option::RelaxedIntegers option during decoding allows change such behavior.

```
	//{'int': 1}
	//(14) : b'\x0e\x00\x00\x00\x10int\x00\x01\x00\x00\x00\x00'
	//                          ^^^ int32
	std::uint8_t data[] = { 0xe, 0x0, 0x0, 0x0, 0x10, 0x69, 0x6e, 0x74, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0 };
	uint8_t const *begin = data;
	uint8_t const* end = begin + sizeof(data) / sizeof(data[0]);
	std::tuple<std::uint64_t> res = bobl::bson::decode<std::uint64_t, bobl::Options<bobl::options::RelaxedIntegers>>(begin, end);
	assert(std::get<0>(res) == 1);
```
bobl::option::FloatOptimizeSize/bobl::option::RelaxedFloats works the same way for floating point types.

[Boost.Fusion](https://www.boost.org/doc/libs/1_50_0/libs/fusion/doc/html/fusion/adapted/adapt_struct.html) adapted structures are encoded/decoded as bson/cbor objects, also called tables, dictionaries, hashes or maps of name-values pairs. By default member of structures decoded/encoded in they declaration order. It is possible to decode such objects encoded in different order if resulting C++ object is default constructible and bobl::options::StructAsDictionary option is specifyed. 

Also decoding ignores extra object members at the end of object if bobl::options::StructAsDictionary option is not used. And any extra members if bobl::options::StructAsDictionary is used. This behavior allows, to certain point, extend protocols without breaking existing implementations. To suppress such behavior bobl::option::ExacMatch can be used. It makes decode throw bobl::InvalidObject exception if any extra object members found during decoding.

#### enums and enum classes
Are encoded/decoded as underlying integer type.


### Requirements
 - c++11 suported compiler (clang 3.6+, gcc 4.8.5+, msvc-14.1+ )
 - [boost](http://www.boost.org) ([MPL](http://www.boost.org/doc/libs/1_66_0/libs/mpl/doc/index.html), [Fusion](http://www.boost.org/doc/libs/1_66_0/libs/fusion/doc/html/), [Uuid](https://www.boost.org/doc/libs/1_67_0/libs/uuid/doc/index.html), [Endian](https://www.boost.org/doc/libs/1_67_0/libs/endian/doc/index.html), [Range](https://www.boost.org/doc/libs/1_67_0/libs/range/doc/html/index.html), [Format](https://www.boost.org/doc/libs/1_67_0/libs/format/))

