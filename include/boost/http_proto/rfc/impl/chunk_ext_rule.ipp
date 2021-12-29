//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_CHUNK_EXT_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_CHUNK_EXT_IPP

#include <boost/http_proto/rfc/chunk_ext_rule.hpp>
#include <boost/http_proto/bnf/token.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/http_proto/rfc/ows_rule.hpp>
#include <boost/http_proto/rfc/quoted_string_rule.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/token_rule.hpp>

namespace boost {
namespace http_proto {

bool
chunk_ext_rule::
parse(
    char const*& it,
    char const* const end,
    error_code& ec,
    reference& t) noexcept
{
    using grammar::parse;

    grammar::token_rule<tchars_t> t0;

    // BWS ';' BWS chunk-ext-name
    if(! grammar::parse_all(
        it, end, ec,
        ows_rule{},
        ';',
        ows_rule{},
        t0))
        return false;
    t.name = *t0;

    // [ BWS "=" BWS ( token / quoted-string ) ]
    if(! grammar::parse_all(
        it, end, ec,
        ows_rule{},
        '=',
        ows_rule{}))
    {
        ec = {};
        t.value = {};
        return true;
    }

    if(it == end)
    {
        ec = grammar::error::incomplete;
        return false;
    }

    if(*it != '\"')
    {
        // token
        if(! grammar::parse_all(
            it, end, ec, t0))
            return false;
        t.value = *t0;
        return true;
    }

    // quoted-string
    quoted_string_rule q;
    if(! parse(it, end, ec, q))
        return false;
    t.value = *q;
    return true;
}

} // http_proto
} // boost

#endif
