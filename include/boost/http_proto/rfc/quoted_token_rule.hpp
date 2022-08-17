//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_QUOTED_TOKEN_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_QUOTED_TOKEN_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

class quoted_token_view
{
    string_view s_;
    std::size_t n_ = 0;

    friend struct quoted_token_rule_t;

    // unquoted
    explicit
    quoted_token_view(
        string_view s) noexcept
        : s_(s)
        , n_(s.size())
    {
    }

    // maybe quoted
    quoted_token_view(
        string_view s,
        std::size_t n) noexcept
        : s_(s)
        , n_(n)
    {
        BOOST_ASSERT(s.size() >= 2);
        BOOST_ASSERT(s.front() == '\"');
        BOOST_ASSERT(s.back() == '\"');
        BOOST_ASSERT(n_ <= s_.size() - 2);
    }

public:
    quoted_token_view() = default;

    quoted_token_view(
        quoted_token_view const&) noexcept = default;

    quoted_token_view& operator=(
        quoted_token_view const&) noexcept = default;

    bool
    has_escapes() const noexcept
    {
        return n_ != s_.size();
    }

    string_view
    raw_value() const noexcept
    {
        return s_;
    }
};

//------------------------------------------------

/** Rule matching quoted-token

    @par Value Type
    @code
    using value_type = quoted_token_view;
    @endcode

    @par Example
    @code
    @endcode

    @par BNF
    @code
    quoted-token    = token / quoted-string

    token           = 1*tchar
    quoted-string   = DQUOTE *( qdtext / quoted-pair ) DQUOTE
    qdtext          = HTAB / SP /%x21 / %x23-5B / %x5D-7E / obs-text
    obs-text        = %x80-FF
    quoted-pair     = "\" ( HTAB / SP / VCHAR / obs-text )

    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7230#section-3.2.6"
        >3.2.6.  Field Value Components (rfc7230)</a>
*/
#ifdef BOOST_HTTP_PROTO_DOCS
constexpr __implementation_defined__ quoted_token_rule;
#else
struct quoted_token_rule_t
{
    using value_type = quoted_token_view;

    BOOST_HTTP_PROTO_DECL
    auto
    parse(
        char const*& it,
        char const* end) const noexcept ->
            result<value_type>;
};

constexpr quoted_token_rule_t quoted_token_rule{};
#endif

} // http_proto
} // boost

#endif
