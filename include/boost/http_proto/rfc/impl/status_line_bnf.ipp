//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_STATUS_LINE_BNF_PP
#define BOOST_HTTP_PROTO_RFC_IMPL_STATUS_LINE_BNF_PP

#include <boost/http_proto/rfc/status_line_bnf.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

char const*
status_line_bnf::
parse(
    char const* start,
    char const* end,
    error_code& ec)
{
/*
    status-line     = HTTP-version SP status-code SP reason-phrase CRLF
*/
    auto it = start;

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

    // SP
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != ' ')
    {
        ec = error::bad_status_line;
        return start;
    }
    ++it;

    // status-code
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(! bnf::is_digit(*it))
    {
        ec = error::bad_status_code;
        return start;
    }
    status_code = 100 * (*it - '0');
    ++it;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(! bnf::is_digit(*it))
    {
        ec = error::bad_status_code;
        return start;
    }
    status_code += 10 * (*it - '0');
    ++it;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(! bnf::is_digit(*it))
    {
        ec = error::bad_status_code;
        return start;
    }
    status_code += 1 * (*it - '0');
    ++it;

    // SP
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != ' ')
    {
        ec = error::bad_status_line;
        return start;
    }
    ++it;

    // reason
    bnf::qpchar_set qs;
    start = it;
    it = qs.skip(it, end);
    reason = string_view(
        start, it - start);

    // CRLF
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != '\r')
    {
        ec = error::bad_status_line;
        return start;
    }
    ++it;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != '\n')
    {
        ec = error::bad_status_line;
        return start;
    }
    ++it;

    return it;
}

} // http_proto
} // boost

#endif
