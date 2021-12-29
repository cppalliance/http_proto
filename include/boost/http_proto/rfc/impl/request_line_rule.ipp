//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_REQUEST_LINE_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_REQUEST_LINE_RULE_IPP

#include <boost/http_proto/rfc/request_line_rule.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/http_proto/rfc/crlf_rule.hpp>
#include <boost/http_proto/rfc/version_rule.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/error.hpp>

namespace boost {
namespace http_proto {

/*
    request-line   = method SP request-target SP HTTP-version CRLF
*/
bool
parse(
    char const*& it,
    char const* const end,
    error_code& ec,
    request_line_rule& t) noexcept
{
    using grammar::parse;

    method_rule t0;
    request_target_rule t1;
    version_rule t2;

    if(! parse(
        it, end, ec,
        t0, ' ',
        t1, ' ',
        t2, crlf_rule{}))
        return false;

    t.m = t0.m;
    t.ms = t0.s;
    t.t = t1.s;
    t.v = t2.v;

    return true;
}

} // http_proto
} // boost

#endif
