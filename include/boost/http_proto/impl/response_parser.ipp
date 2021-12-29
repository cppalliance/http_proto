//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_PARSER_IPP

#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/http_proto/rfc/status_line_rule.hpp>

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
    char* const start,
    char const* const end,
    error_code& ec) noexcept
{
    using grammar::parse;

    status_line_rule t;
    char const* it = start;
    if(! parse(it, end, ec, t))
        return start;

    m_.version = t.v;
    status_ = t.status_code;
    return start + (it - start);
}

void
response_parser::
finish_header(
    error_code& ec)
{
    ec = {};

    // https://tools.ietf.org/html/rfc7230#section-3.3
    if((status_ /  100 == 1) || // 1xx e.g. Continue
        status_ == 204 ||       // No Content
        status_ == 304)         // Not Modified
    {
        // Content-Length may be present, but we
        // treat the message as not having a body.
        m_.skip_body = true;
        return;
    }
    else if(m_.content_len.has_value())
    {
        if(*m_.content_len > 0)
        {
            //has_body_ = true;
            state_ = state::body;

            if( cfg_.body_limit.has_value() &&
                *m_.content_len > *cfg_.body_limit)
            {
                ec = error::body_limit;
                return;
            }
        }
    }
}

} // http_proto
} // boost

#endif
