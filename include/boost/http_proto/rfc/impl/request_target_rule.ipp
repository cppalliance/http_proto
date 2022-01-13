//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_REQUEST_TARGET_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_REQUEST_TARGET_RULE_IPP

#include <boost/http_proto/rfc/request_target_rule.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/error.hpp>

namespace boost {
namespace http_proto {

/*
    request-target  = origin-form /
                      absolute-form /
                      authority-form /
                      asterisk-form
*/
void
request_target_rule::
parse(
    char const*& it,
    char const* const end,
    error_code& ec,
    request_target_rule& t) noexcept
{
    if(it == end)
    {
        ec = grammar::error::incomplete;
        return;
    }

    auto const start = it;

    it = grammar::find_if_not(
        it, end, pchars);

    if(it == start)
    {
        // empty target
        ec = grammar::error::syntax;
        return;
    }

    t.s = string_view(
        start, it - start);
}

} // http_proto
} // boost

#endif
