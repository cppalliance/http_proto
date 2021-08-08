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
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

request_parser::
request_parser(
    context& ctx) noexcept
    : basic_parser(ctx)
    , method_(http_proto::method::unknown)
    , n_method_(0)
    , n_target_(0)
{
}

request_view
request_parser::
header() const noexcept
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

char*
request_parser::
parse_start_line(
    char* const start,
    char const* const end,
    error_code& ec)
{
/*
    request-line   = method SP request-target SP HTTP-version CRLF
*/

    auto it = start;

    // method
    it = parse_method(
        it, end, ec);
    if(ec)
        return start;

    // SP
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != ' ')
    {
        ec = error::bad_method;
        return start;
    }
    ++it;

    // request-target
    it = parse_target(
        it, end, ec);
    if(ec)
        return start;

    // SP
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != ' ')
    {
        ec = error::bad_method;
        return start;
    }
    ++it;

    // HTTP-version
    it = parse_version(
        it, end, ec);
    if(ec)
        return start;

    // CRLF
    if(end - it < 2)
    {
        ec = error::need_more;
        return start;
    }
    if( it[0] != '\r' ||
        it[1] != '\n')
    {
        ec = error::bad_version;
        return start;
    }
    it += 2;
    return it;
}

char*
request_parser::
parse_method(
    char* const start,
    char const* const end,
    error_code& ec)
{
    tchar_set ts;

    // token
    auto it = ts.skip(start, end);
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(it == start)
    {
        // empty method
        ec = error::bad_method;
        return start;
    }
    string_view s(
        start, it - start);
    method_ = string_to_method(s);
    n_method_ = static_cast<
        off_t>(s.size());
    return it;
}

char*
request_parser::
parse_target(
    char* const start,
    char const* const end,
    error_code& ec)
{
    detail::pchar_set ps;

    // target
    auto it = ps.skip(
        start, end);;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(it == start)
    {
        // empty target
        ec = error::bad_target;
        return start;
    }
    string_view s(
        start, it - start);
    n_target_ = static_cast<
        off_t>(s.size());
    return it;
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
