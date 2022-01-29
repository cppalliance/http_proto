//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP

#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/version.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/http_proto/rfc/request_line_rule.hpp>
#include <boost/url/grammar/parse.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

request_parser::
request_parser(
    context& ctx) noexcept
    : parser(ctx)
    , method_(http_proto::method::unknown)
    , method_len_(0)
    , target_len_(0)
{
}

request_view
request_parser::
get() const noexcept
{
    request_view::ctor_params init;
    init.cbuf = buf_;
    init.buf_len = cap_;
    init.start_len = m_.start_len;
    init.end_pos = m_.start_len +
        m_.fields_len /* - 2 */;
    init.count = m_.count;
    init.method_len = method_len_;
    init.target_len = target_len_;
    init.method = method_;
    init.version = m_.version;
    return request_view(init);
}

//------------------------------------------------

char*
request_parser::
parse_start_line(
    char* const start,
    char const* const end,
    error_code& ec) noexcept
{
    request_line_rule t;
    char const* it = start;
    if(! grammar::parse(
        it, end, ec, t))
        return start;

    method_ = t.m;
    method_len_ = static_cast<
        off_t>(t.ms.size());
    target_len_ = static_cast<
        off_t>(t.t.size());
    m_.version = t.v;
    return start + (it - start);
}

void
request_parser::
finish_header(
    error_code& ec)
{
    // https://tools.ietf.org/html/rfc7230#section-3.3
    if(m_.skip_body)
    {
        ec = error::end_of_message;
        state_ = state::end_of_message;
        return;
    }
    if(m_.content_len.has_value())
    {
        if( cfg_.body_limit.has_value() &&
            *m_.content_len > *cfg_.body_limit)
        {
            ec = error::body_limit;
            return;
        }
        if(*m_.content_len > 0)
        {
            state_ = state::body;
            return;
        }
        ec = error::end_of_message;
        state_ = state::end_of_message;
        return;
    }
    else if(m_.got_chunked)
    {
        state_ = state::body;
        return;
    }
    ec = error::end_of_message;
    state_ = state::end_of_message;
}

} // http_proto
} // boost

#endif
