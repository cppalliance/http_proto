//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_REQUEST_LINE_BNF_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_REQUEST_LINE_BNF_IPP

#include <boost/http_proto/rfc/request_line_bnf.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>
#include <boost/http_proto/error.hpp>

namespace boost {
namespace http_proto {

char const*
request_line_bnf::
parse(
    char const* start,
    char const* end,
    error_code& ec)
{
    // request-line   = method SP request-target SP HTTP-version CRLF

    auto it = start;

    // method
    it = parse_method(
        it, end, ec);
    if(ec.failed())
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
    if(ec.failed())
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
    {
        it = bnf::detail::parse_http_version(
            version, it, end, ec);
        if(ec.failed())
            return start;
        if( version != 10 &&
            version != 11)
        {
            ec = error::bad_version;
            return start;
        }
    }

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

char const*
request_line_bnf::
parse_method(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    bnf::tchar_set ts;

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
    method = string_view(
        start, it - start );
    return it;
}

char const*
request_line_bnf::
parse_target(
    char const* const start,
    char const* const end,
    error_code& ec)
{
/*
    request-target  = origin-form
                    / absolute-form
                    / authority-form
                    / asterisk-form
*/
    bnf::detail::pchar_set ps;

    // target
    auto it = ps.skip(
        start, end);
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(it == start)
    {
        // empty target
        ec = error::bad_request_target;
        return start;
    }
    target = string_view(
        start, it - start );
    return it;
}

} // http_proto
} // boost

#endif
