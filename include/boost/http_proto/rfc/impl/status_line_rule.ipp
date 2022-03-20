//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_STATUS_LINE_RULE_PP
#define BOOST_HTTP_PROTO_RFC_IMPL_STATUS_LINE_RULE_PP

#include <boost/http_proto/rfc/status_line_rule.hpp>
#include <boost/http_proto/rfc/crlf_rule.hpp>
#include <boost/http_proto/rfc/version_rule.hpp>

namespace boost {
namespace http_proto {

/*
    status-line     = HTTP-version SP status-code SP reason-phrase CRLF
*/
void
status_line_rule::
parse(
    char const*& it,
    char const* end,
    error_code& ec,
    status_line_rule& t) noexcept
{
    version_rule t0;
    status_code_rule t1;
    reason_phrase_rule t2;

    if(! grammar::parse(
        it, end, ec,
        t0, ' ',
        t1, ' ',
        t2, crlf_rule{}))
        return;

    t.v = t0.v;
    t.status_int = static_cast<
        unsigned short>(t1.v);
    t.reason = t2.s;
    return;
}

} // http_proto
} // boost

#endif
