//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_CHUNK_EXT_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_CHUNK_EXT_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/bnf/sequence.hpp>

namespace boost {
namespace http_proto {

/** Rule for chunk-ext

    @par BNF
    @code
    chunk-ext      = *( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )

    chunk-ext-name = token
    chunk-ext-val  = token / quoted-string
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-4.1.1"
        >4.1.1. Chunk Extensions (rfc7230)</a>
    @li <a href="https://www.rfc-editor.org/errata/eid4667"
        >Errata ID: 4667 (rfc-editor)</a>
*/
struct chunk_ext_rule
{
    struct reference
    {
        string_view name;
        string_view value;
    };

    struct value_type
    {
        std::string name;
        std::string value;

        value_type() = default;

        value_type(
            reference const& t)
            : name(t.name)
            , value(t.value)
        {
        }

        operator reference() const noexcept
        {
            return reference{name, value};
        }
    };

    BOOST_HTTP_PROTO_DECL
    static
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        reference& t) noexcept;

    static
    bool
    begin(
        char const*& it,
        char const* end,
        error_code& ec,
        reference& t) noexcept
    {
        return increment(
            it, end, ec, t);
    }

    static
    bool
    increment(
        char const*& it,
        char const* end,
        error_code& ec,
        reference& t) noexcept
    {
        auto const start = it;
        if(parse(it, end, ec, t))
            return true;
        if(it == start)
            ec = grammar::error::end;
        return false;
    }
};

} // http_proto
} // boost

#endif
