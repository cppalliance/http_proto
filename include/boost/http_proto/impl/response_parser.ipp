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
#include <boost/http_proto/bnf/status_line.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

response_parser::
response_parser(
    context& ctx) noexcept
    : parser(ctx)
{
}

char*
response_parser::
parse_start_line(
    char* first,
    char const* const last,
    error_code& ec)
{
    bnf::status_line p;
    auto it = p.parse(
        first, last, ec);
    if(ec)
        return first;
    switch(p.value().version)
    {
    case 10:
        m_.version = version::http_1_0;
        break;
    default:
    case 11:
        m_.version = version::http_1_1;
        break;
    }
    status_ = p.value().status_code;
    
    return first + (it - first);
}

void
response_parser::
finish_header(
    error_code& ec)
{
    ec = {};

    // https://tools.ietf.org/html/rfc7230#section-3.3
    if((status_ / 100 == 1) ||  // 1xx e.g. Continue
        status_ == 204 ||       // No Content
        status_ == 304)         // Not Modified
    {
        // Content-Length may be present, but we
        // treat the message as not having a body.
        m_.skip_body = true;
        return;
    }
}

} // http_proto
} // boost

#endif
