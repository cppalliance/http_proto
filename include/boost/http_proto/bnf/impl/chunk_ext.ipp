//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_EXT_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_EXT_IPP

#include <boost/http_proto/bnf/chunk_ext.hpp>
#include <boost/http_proto/bnf/quoted_string.hpp>
#include <boost/http_proto/bnf/token.hpp>
#include <boost/http_proto/error.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
chunk_ext_elem::
parse(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    if(start == end)
    {
        ec = error::need_more;
        return start;
    }
    auto it = start;
    // ";"
    if(*it != ';')
    {
        // expected ';"
        ec = error::syntax;
        return start;
    }
    ++it;
    // token
    token t;
    it = t.parse(
        it, end, ec);
    if(ec.failed())
        return start;
    v_.name = t.value();
    if(it == end)
        return it;
    // [ "=" ( token / quoted-string ) ]
    if(*it != '=')
        return it;
    ++it;
    // token
    it = t.parse(
        it, end, ec);
    if(! ec)
    {
        v_.value = t.value();
        return it;
    }
    ec = {};
    // quoted-string
    quoted_string q;
    it = q.parse(
        it, end, ec);
    if(! ec)
    {
        v_.value = q.value();
        return it;
    }
    // expected token or
    // quoted-string
    ec = error::syntax;
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
