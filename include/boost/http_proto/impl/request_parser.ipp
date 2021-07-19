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

//------------------------------------------------

request_parser::
request_parser() noexcept
    : basic_parser()
    , method_(http_proto::method::unknown)
    , version_(0)
    , n_method_(0)
    , n_target_(0)
{
}

request_view
request_parser::
get() const noexcept
{
    return request_view(
        buffer_,
        size_,
        n_method_,
        n_target_,
        method_,
        version_);
}

//------------------------------------------------

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

    // method
    {
        string_view s;
        parse_method(p, last, s, ec);
        if(ec)
            return;
        method_ = string_to_method(s);
        n_method_ = static_cast<
            unsigned short>(s.size());
    }

    // request-target
    {
        string_view s;
        parse_target(p, last, s, ec);
        if(ec)
            return;
        n_target_ = static_cast<
            unsigned short>(s.size());
    }

    // HTTP-version
    {
        int v = 0;
        parse_version(p, last, v, ec);
        if(ec)
            return;
        if(v != 10 && v != 11)
        {
            ec = error::bad_version;
            return;
        }
        version_ = v;
        if(version_ == 11)
            f_ |= flagHTTP11;
    }

    // CRLF
    if(last - p < 2)
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

    // at this point the request line is parsed
    in = p;
}

void
request_parser::
parse_method(
    char const*& it, char const* last,
    string_view& result, error_code& ec)
{
    // token SP
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
    // target SP
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
