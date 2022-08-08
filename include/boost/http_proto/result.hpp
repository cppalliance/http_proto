//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESULT_HPP
#define BOOST_HTTP_PROTO_RESULT_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_code.hpp>
#include <boost/url/result.hpp>

namespace boost {
namespace http_proto {

/** The type of result used by the library
*/
template<class T>
using result = urls::result<T>;

} // http_proto
} // boost

#endif
