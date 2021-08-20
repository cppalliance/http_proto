//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_VERSION_HPP
#define BOOST_HTTP_PROTO_VERSION_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>
#include <iosfwd>

namespace boost {
namespace http_proto {

/** Constants representing HTTP versions.

    Only versions 1.0 and 1.1 are recognized.
*/
enum class version : char
{
    http_1_0 = 10,
    http_1_1 = 11
};

/** Return the serialized string representing the HTTP version
*/
BOOST_HTTP_PROTO_DECL
string_view
to_string(version v) noexcept;

BOOST_HTTP_PROTO_DECL
std::ostream&
operator<<(std::ostream& os, version v);

} // http_proto
} // boost

#endif
