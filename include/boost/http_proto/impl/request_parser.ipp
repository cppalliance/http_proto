//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP

#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/version.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/request_line.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>

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
    return request_view(
        buf_,
        cap_,
        m_.count,
        m_.start_len,
        m_.fields_len - 2,
        method_len_,
        target_len_,
        method_,
        m_.version);
}

//------------------------------------------------

char*
request_parser::
parse_start_line(
    char* const start,
    char const* const end,
    error_code& ec)
{
    bnf::request_line p;
    auto it = p.parse(
        start, end, ec);
    if(ec.failed())
        return start + (it - start);
    method_ = string_to_method(
        p.value().method);
    method_len_ = static_cast<
        off_t>(p.value().method.size());
    target_len_ = static_cast<
        off_t>(p.value().target.size());
    switch(p.value().version)
    {
    case 10:
        m_.version = http_proto::version::http_1_0;
        break;
    default:
    case 11:
        m_.version = http_proto::version::http_1_1;
        break;
    }
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
        if( cfg_.body_limit > 0 && // optional?
            m_.content_len >
                cfg_.body_limit)
        {
            ec = error::body_limit;
            return;
        }
        if(*m_.content_len > 0)
        {
            state_ = state::payload;
            return;
        }
        ec = error::end_of_message;
        state_ = state::end_of_message;
        return;
    }
    else if(m_.got_chunked)
    {
        state_ = state::payload;
        return;
    }
    ec = error::end_of_message;
    state_ = state::end_of_message;
}

} // http_proto
} // boost

#endif
