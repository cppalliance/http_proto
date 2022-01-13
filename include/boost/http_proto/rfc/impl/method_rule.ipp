//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_METHOD_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_METHOD_RULE_IPP

#include <boost/http_proto/rfc/method_rule.hpp>
#include <boost/url/grammar/error.hpp>

namespace boost {
namespace http_proto {

void
method_rule::
parse(
    char const*& it,
    char const* end,
    error_code& ec,
    method_rule& t) noexcept
{
    token t0;
    if(! grammar::parse(
        it, end, ec, t0))
        return;
    t.s = *t0;
    t.m = string_to_method(t.s);
}

} // http_proto
} // boost

#endif
