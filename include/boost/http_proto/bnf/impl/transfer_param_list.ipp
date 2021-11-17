//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_TRANSFER_PARAM_LIST_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_TRANSFER_PARAM_LIST_IPP

#include <boost/http_proto/bnf/transfer_param_list.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/quoted_string.hpp>
#include <boost/http_proto/bnf/token.hpp>

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
    // OWS
    ws_set ws_;
    auto it =
        ws_.skip(start, end);
    // ";"
    it = consume(';',
        it, end, ec);
    if(ec.failed())
        return start;
    // OWS
    it = ws_.skip(it, end);
    // token
    token t;
    it = t.parse(it, end, ec);
    if(ec.failed())
        return it;
    v_.name = t.value();
    // OWS
    it = ws_.skip(it, end);
    // "="
    it = consume('=',
        it, end, ec);
    if(ec.failed())
        return start;
    // OWS
    it = ws_.skip(it, end);
    // ( token / quoted-string )
    it = t.parse(it, end, ec);
    if(! ec)
    {
        // token
        v_.value = t.value();
        return it;
    }
    ec = {};
    quoted_string q;
    it = q.parse(it, end, ec);
    if(! ec)
    {
        // quoted-string
        v_.value = q.value();
        return it;
    }
    // value must be present
    // https://www.rfc-editor.org/errata/eid4839
    return start;
}

} // bnf
} // http_proto
} // boost

#endif
