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
#include <boost/http_proto/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

void
request_parser::
parse_start_line(
    char const*& in,
    char const* const last,
    error_code& ec)
{
/*
    request-line   = method SP request-target SP HTTP-version CRLF
    method         = token
*/
    auto p = in;

    string_view method;
    parse_method(p, last, method, ec);
    if(ec)
        return;

    string_view target;
    parse_target(p, last, target, ec);
    if(ec)
        return;

    int version = 0;
    parse_version(p, last, version, ec);
    if(ec)
        return;
    if(version < 10 || version > 11)
    {
        ec = error::bad_version;
        return;
    }

    if(p + 2 > last)
    {
        ec = error::need_more;
        return;
    }
    if(p[0] != '\r' || p[1] != '\n')
    {
        ec = error::bad_version;
        return;
    }
    p += 2;

    if(version >= 11)
        f_ |= flagHTTP11;

    // VFALCO TODO
    /*
    this->on_request_impl(string_to_verb(method),
        method, target, version, ec);
    if(ec)
        return;
    */

    in = p;
}

void
request_parser::
parse_method(
    char const*& it, char const* last,
    string_view& result, error_code& ec)
{
    // parse token SP
    auto const first = it;
    for(;; ++it)
    {
        if(it + 1 > last)
        {
            ec = error::need_more;
            return;
        }
        if(! detail::is_token_char(*it))
            break;
    }
    if(it + 1 > last)
    {
        ec = error::need_more;
        return;
    }
    if(*it != ' ')
    {
        ec = error::bad_method;
        return;
    }
    if(it == first)
    {
        // cannot be empty
        ec = error::bad_method;
        return;
    }
    result = string_view(
        first, it++ - first);
}

void
request_parser::
parse_target(
    char const*& it, char const* last,
    string_view& result, error_code& ec)
{
    // parse target SP
    auto const first = it;
    for(;; ++it)
    {
        if(it + 1 > last)
        {
            ec = error::need_more;
            return;
        }
        if(! detail::is_pathchar(*it))
            break;
    }
    if(it + 1 > last)
    {
        ec = error::need_more;
        return;
    }
    if(*it != ' ')
    {
        ec = error::bad_target;
        return;
    }
    if(it == first)
    {
        // cannot be empty
        ec = error::bad_target;
        return;
    }
    result = string_view(
        first, it++ - first);
}

void
request_parser::
finish_header(
    error_code& ec)
{
    (void)ec;
}

} // http_proto
} // boost

#endif
