//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/version.hpp>
#include <ostream>

namespace boost {
namespace http_proto {

core::string_view
to_string(
    version v) noexcept
{
    switch(v)
    {
    case version::http_1_0:
        return "HTTP/1.0";
    default:
    case version::http_1_1:
        return "HTTP/1.1";
    }
}

std::ostream&
operator<<(
    std::ostream& os,
    version v)
{
    os << to_string(v);
    return os;
}

} // http_proto
} // boost
