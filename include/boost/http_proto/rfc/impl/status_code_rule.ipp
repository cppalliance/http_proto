//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_STATUS_CODE_RULE_PP
#define BOOST_HTTP_PROTO_RFC_IMPL_STATUS_CODE_RULE_PP

#include <boost/http_proto/rfc/status_line_rule.hpp>
#include <boost/http_proto/rfc/crlf_rule.hpp>
#include <boost/http_proto/rfc/version_rule.hpp>

namespace boost {
namespace http_proto {

/*
    status-code     = 3DIGIT
*/
bool
parse(
    char const*& it0,
    char const* end,
    error_code& ec,
    status_code_rule& t) noexcept
{
    using grammar::parse;

    auto const dig =
        [](char c) -> int
        {
            unsigned char uc(c - '0');
            if(uc > 9)
                return -1;
            return uc;
        };

    auto it = it0;
    int v;
        
    if(it == end)
    {
        ec = grammar::error::incomplete;
        return false;
    }
    v = dig(*it);
    if(v == -1)
    {
        ec = error::bad_status_code;
        return false;
    }
    t.v = 100 * v;
    ++it;
       
    if(it == end)
    {
        ec = grammar::error::incomplete;
        return false;
    }
    v = dig(*it);
    if(v == -1)
    {
        ec = error::bad_status_code;
        return false;
    }
    t.v = t.v + (10 * v);
    ++it;

    if(it == end)
    {
        ec = grammar::error::incomplete;
        return false;
    }
    v = dig(*it);
    if(v == -1)
    {
        ec = error::bad_status_code;
        return false;
    }
    t.v = t.v + v;
    ++it;

    t.s = string_view(it0, it - it0);
    t.st = int_to_status(t.v);
    return true;
}

} // http_proto
} // boost

#endif
