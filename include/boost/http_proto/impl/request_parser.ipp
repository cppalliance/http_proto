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
request_parser(
    context& ctx) noexcept
    : basic_parser(ctx)
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
        parsed_,
        n_method_,
        n_target_,
        method_,
        version_);
}

//------------------------------------------------

bool
request_parser::
parse_start_line(
    char*& first,
    char const* const last,
    error_code& ec)
{
/*
    request-line   = method SP request-target SP HTTP-version CRLF
    method         = token
*/
    auto it = first;

    // method
    if(! parse_method(it, last, ec))
        return false;

    // request-target
    if(! parse_target(it, last, ec))
        return false;

    // HTTP-version
    if(! parse_version(
            it, last, version_, ec))
        return false;

    // CRLF
    if(last - it < 2)
        return false;
    if(it[0] != '\r' || it[1] != '\n')
    {
        ec = error::bad_version;
        return false;
    }
    first = it + 2;
    return true;
}

bool
request_parser::
parse_method(
    char*& first,
    char const* last,
    error_code& ec)
{
    // token SP
    auto it = first;
    for(;; ++it)
    {
        if(it == last)
            return false;
        if(! detail::is_token_char(*it))
            break;
    }
    if(it == last)
        return false;
    if(*it != ' ')
    {
        // bad token char
        ec = error::bad_method;
        return false;
    }
    if(it == first)
    {
        // empty method
        ec = error::bad_method;
        return false;
    }
    string_view s(
        first, it++ - first);
    method_ = string_to_method(s);
    n_method_ = static_cast<
        off_t>(s.size());
    first = it;
    return true;
}

bool
request_parser::
parse_target(
    char*& first,
    char const* last,
    error_code& ec)
{
    // target SP
    auto it = first;
    for(;; ++it)
    {
        if(it == last)
            return false;
        if(! detail::is_pathchar(*it))
            break;
    }
    if(it == last)
        return false;
    if(*it != ' ')
    {
        // bad path char
        ec = error::bad_target;
        return false;
    }
    if(it == first)
    {
        // empty target
        ec = error::bad_target;
        return false;
    }
    string_view s(
        first, it++ - first);
    n_target_ = static_cast<
        off_t>(s.size());
    first = it;
    return true;
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
