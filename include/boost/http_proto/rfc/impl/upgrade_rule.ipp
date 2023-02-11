//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_UPGRADE_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_UPGRADE_RULE_IPP

#include <boost/http_proto/rfc/upgrade_rule.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/url/grammar/parse.hpp>

namespace boost {
namespace http_proto {

auto
upgrade_protocol_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        result<value_type>
{
    value_type t;
    // token
    {
        auto rv = grammar::parse(
            it, end, token_rule);
        if(! rv)
            return rv.error();
        t.name = *rv;
    }
    // [ "/" token ]
    if( it == end ||
        *it != '/')
        return t;
    ++it;
    auto rv = grammar::parse(
        it, end, token_rule);
    if(! rv)
        return rv.error();
    t.version = *rv;
    return t;
}

} // http_proto
} // boost

#endif
