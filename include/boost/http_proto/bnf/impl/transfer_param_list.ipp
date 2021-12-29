//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_IMPL_TRANSFER_PARAM_LIST_IPP
#define BOOST_HTTP_PROTO_RULE_IMPL_TRANSFER_PARAM_LIST_IPP

#include <boost/http_proto/bnf/transfer_param_list.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/token.hpp>
#include <boost/http_proto/rfc/quoted_string_rule.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
transfer_param_elem::
parse(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    using grammar::parse;

    // OWS
    auto it = grammar::find_if_not(
        start, end, ws);
    // ";"
    it = consume(';',
        it, end, ec);
    if(ec.failed())
        return start;
    // OWS
    it = grammar::find_if_not(
        it, end, ws);
    // token
    token t;
    it = t.parse(it, end, ec);
    if(ec.failed())
        return it;
    v_.name = t.value();
    // OWS
    it = grammar::find_if_not(
        it, end, ws);
    // "="
    it = consume('=',
        it, end, ec);
    if(ec.failed())
        return start;
    // OWS
    it = grammar::find_if_not(
        it, end, ws);
    // ( token / quoted-string )
    it = t.parse(it, end, ec);
    if(! ec)
    {
        // token
        v_.value = t.value();
        return it;
    }
    ec = {};
    quoted_string_rule q;
    if(! parse(it, end, ec, q))
        return start;
    v_.value = *q;

    // value must be present
    // https://www.rfc-editor.org/errata/eid4839
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
