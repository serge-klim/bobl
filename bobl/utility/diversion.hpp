// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <string>

#ifdef BOBL_PREFER_BOOST_TYPES__
#define BOBL_PREFER_BOOST_STRING_VIEW__
#define BOBL_PREFER_BOOST_OPTIONAL__
#define BOBL_PREFER_BOOST_VARIANT__
#endif //BOBL_PREFER_BOOST_TYPES__

#if defined(BOBL_PREFER_BOOST_STRING_VIEW__) || !defined(__cpp_lib_string_view)
#include <boost/utility/string_view.hpp>
namespace diversion { using boost::string_view; }
#else
#include <string_view>
namespace diversion { using std::string_view; }
#endif
#if defined(BOBL_PREFER_BOOST_OPTIONAL__) || !defined(__cpp_lib_optional)
#include <boost/optional.hpp>
namespace diversion { using boost::optional; using boost::make_optional; const auto nullopt = boost::none;}
#else
#include<optional>
namespace diversion { using std::optional; using std::make_optional; using std::nullopt;}
#endif

#if defined(BOBL_PREFER_BOOST_VARIANT__) || !defined(__cpp_lib_variant)
#include <boost/variant.hpp>
namespace diversion { 
using boost::variant; using boost::get;
//using boost::apply_visitor; 
template<typename ...Args> auto visit(Args&& ...args) -> decltype(boost::apply_visitor(std::forward<Args>(args)...)) { return boost::apply_visitor(std::forward<Args>(args)...); }
}
#else
#include<variant>
namespace diversion { using std::variant; using std::visit; using std::get;}
#endif

namespace diversion {

using std::to_string; /*boost::lexical_cast<std::string>*/

}/*diversion*/
