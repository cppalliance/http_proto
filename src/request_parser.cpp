//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/request_parser.hpp>

namespace boost {
namespace http_proto {

request_parser::
request_parser(
    context& ctx)
    : parser(
        ctx,
        detail::kind::request)
{
}

request_view
request_parser::
get() const
{
    return request_view(
        safe_get_header());
}

} // http_proto
} // boost
