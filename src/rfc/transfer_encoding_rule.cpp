//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include "transfer_encoding_rule.hpp"

#include <boost/http_proto/rfc/token_rule.hpp>
#include <boost/http_proto/rfc/detail/rules.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/parse.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

namespace detail {

/*
    tparams = *tparam
    tparam  = OWS ";" OWS token BWS "=" BWS ( token / quoted-string )
*/

struct tparam_rule_t
{
    using value_type =
        transfer_encoding::param;

    auto
    parse(
        char const*& it,
        char const* end) const noexcept ->
            system::result<value_type>
    {
        value_type t;
        auto it0 = it;
        // OWS
        it = grammar::find_if_not(
            it, end, ws);
        // ";"
        if(it == end)
        {
            it = it0;
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if(*it != ';')
        {
            it = it0;
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::mismatch);
        }
        ++it;
        // OWS
        it = grammar::find_if_not(
            it, end, ws);
        // token
        {
            auto rv = grammar::parse(
                it, end, token_rule);
            if(! rv)
                return rv.error();
            t.key = *rv;
        }
        // BWS
        it = grammar::find_if_not(
            it, end, ws);
        // "="
        if(it == end)
        {
            it = it0;
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if(*it != '=')
        {
            it = it0;
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::syntax);
        }
        ++it;
        // BWS
        it = grammar::find_if_not(
            it, end, ws);
        // quoted-token
        {
            auto rv = grammar::parse(
                it, end, quoted_token_rule);
            if(! rv)
                return rv.error();
            t.value = *rv;
        }
        return t;
    }
};

constexpr tparam_rule_t tparam_rule{};

//------------------------------------------------

auto
transfer_encoding_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        system::result<value_type>
{
    value_type t;
    {
        // token
        auto rv = grammar::parse(
            it, end, token_rule);
        if(! rv)
            return rv.error();
        t.str = *rv;

        // These can't have tparams
        if(grammar::ci_is_equal(
            t.str, "chunked"))
        {
            t.id = transfer_encoding::chunked;
            return t;
        }
        if(grammar::ci_is_equal(
            t.str, "compress"))
        {
            t.id = transfer_encoding::compress;
            return t;
        }
        if(grammar::ci_is_equal(
            t.str, "deflate"))
        {
            t.id = transfer_encoding::deflate;
            return t;
        }
        if(grammar::ci_is_equal(
            t.str, "gzip"))
        {
            t.id = transfer_encoding::gzip;
            return t;
        }
    }
//  *( OWS ";" OWS token BWS "=" BWS ( token / quoted-string )
    {
        auto rv = grammar::parse(it, end,
            grammar::range_rule(
                detail::tparam_rule));
        if(! rv)
            return rv.error();
        t.params = std::move(*rv);
    }
    return t;
}
} // detail


} // http_proto
} // boost
