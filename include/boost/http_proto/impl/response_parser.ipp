//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_PARSER_IPP

#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

response_parser::
response_parser(
    context& ctx) noexcept
    : basic_parser(ctx)
{
}

bool
response_parser::
parse_start_line(
    char*& in,
    char const* const last,
    error_code& ec)
{
    (void)ec;
    (void)last;
    (void)in;
    return false;
}

void
response_parser::
finish_header(
    error_code& ec)
{
    (void)ec;
}

} // http_proto
} // boost

#endif
