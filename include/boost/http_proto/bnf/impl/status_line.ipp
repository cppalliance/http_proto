//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_STATUS_LINE_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_STATUS_LINE_IPP

#include <boost/http_proto/bnf/status_line.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
status_line::
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
        char v;
        it = detail::parse_http_version(
            v, it, end, ec);
        if(ec.failed())
            return start;
        switch(v)
        {
        case 10:
        case 11:
            v_.version = v;
            break;
        default:
            ec = error::bad_version;
            break;
        }
        if(ec.failed())
            return start;
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
    if(! is_digit(*it))
    {
        ec = error::bad_status_code;
        return start;
    }
    v_.status_code = 100 * (*it - '0');
    ++it;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(! is_digit(*it))
    {
        ec = error::bad_status_code;
        return start;
    }
    v_.status_code += 10 * (*it - '0');
    ++it;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(! is_digit(*it))
    {
        ec = error::bad_status_code;
        return start;
    }
    v_.status_code += 1 * (*it - '0');
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
    v_.reason = string_view(
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

} // bnf
} // http_proto
} // boost

#endif
