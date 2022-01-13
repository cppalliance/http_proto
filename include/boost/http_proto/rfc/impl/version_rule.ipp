//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_VERSION_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_VERSION_RULE_IPP

#include <boost/http_proto/rfc/version_rule.hpp>

namespace boost {
namespace http_proto {

/*
    HTTP-version = "HTTP/" DIGIT "." DIGIT
*/
void
version_rule::
parse(
    char const*& it0,
    char const* end,
    error_code& ec,
    version_rule& t) noexcept
{
    auto it = it0;

    if(it == end)
    {
        ec = grammar::error::incomplete;
        return;
    }
    auto n = end - it;
    if( n > 7)
        n = 7;
    if(std::memcmp(it,
        "HTTP/1.", n) != 0)
    {
        // fail fast
        ec = grammar::error::syntax;
        return;
    }
    it += n;
    if(it == end)
    {
        ec = grammar::error::incomplete;
        return;
    }
    switch(*it)
    {
    case '0':
        t.v = http_proto::version::http_1_0;
        break;
    case '1':
        t.v = http_proto::version::http_1_1;
        break;
    default:
        ec = grammar::error::syntax;
        return;
    }
    ++it;
    it0 = it;
}

} // http_proto
} // boost

#endif
