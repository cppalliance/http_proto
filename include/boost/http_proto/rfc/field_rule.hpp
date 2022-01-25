//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_FIELD_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_FIELD_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/grammar/parse_tag.hpp>
#include <string>

namespace boost {
namespace http_proto {

/** Replace obs-fold with spaces
*/
BOOST_HTTP_PROTO_DECL
void
replace_obs_fold(
    char *start,
    char const* end) noexcept;

//------------------------------------------------

/** Rule for header-field

    @par BNF
    @code
    header-field   = field-name ":" OWS field-value OWS

    field-name      = token
    field-value     = *( field-content / obs-fold )
    field-content   = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]

    obs-fold        = OWS CRLF 1*( SP / HTAB )
                    ; obsolete line folding
                    ; see Section 3.2.4

    token           = 1*tchar
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2"
        >3.2. Header Fields (rfc7230)</a>
    @li <a href="https://www.rfc-editor.org/errata/eid4189"
        >RFC 7230 errata</a>
*/
struct field_rule
{
    struct reference
    {
        string_view name;
        string_view value;
        bool has_obs_fold = false;

        reference()
          : has_obs_fold(false)
        {}

        reference(string_view name_,
                  string_view value_,
                  bool has_obs_fold_ = false)
          : name(name_),
            value(value_),
            has_obs_fold(has_obs_fold_)
        {}
    };

    struct value_type
    {
        std::string name;
        std::string value;
        bool has_obs_fold = false;

        value_type() = default;

        value_type(
            reference const& t)
            : name(t.name)
            , value(t.value)
            , has_obs_fold(t.has_obs_fold)
        {
        }

        operator reference() const noexcept
        {
            return reference{
                name, value, has_obs_fold};
        }
    };

    reference v;

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        field_rule& t) noexcept
    {
        parse(it, end, ec, t);
    }

    static
    bool
    begin(
        char const*& it,
        char const* end,
        error_code& ec,
        reference& t) noexcept
    {
        field_rule t0;
        parse(it, end, ec, t0);
        if(ec.failed())
            return false;
        t = t0.v;
        return true;
    }

    static
    bool
    increment(
        char const*& it,
        char const* end,
        error_code& ec,
        reference& t) noexcept
    {
        return begin(it, end, ec, t);
    }

private:
    BOOST_HTTP_PROTO_DECL
    static
    void
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        field_rule& t) noexcept;
};

} // http_proto
} // boost

#endif
