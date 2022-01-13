//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_IMPL_QUOTED_STRING_RULE_IPP
#define BOOST_HTTP_PROTO_RULE_IMPL_QUOTED_STRING_RULE_IPP

#include <boost/http_proto/rfc/quoted_string_rule.hpp>
#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/parse.hpp>

namespace boost {
namespace http_proto {

void
quoted_string_rule::
parse(
    char const*& it,
    char const* const end,
    error_code& ec,
    quoted_string_rule& t) noexcept
{
/*
    qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
    obs-text        = %x80-FF
*/
    struct qdtext_t : grammar::lut_chars
    {
        // workaround for
        // C++11 constexpr
        struct lambda
        {
            constexpr
            bool
            operator()(char c) const noexcept
            {
                return
                    c == 9 || c == 32 || c == 33 ||
                    (c >= 0x23 && c <= 0x5b) ||
                    (c >= 0x5d && c <= 0x7e) ||
                    (static_cast<
                        unsigned char>(c) >= 128);
            }
        };

        constexpr
        qdtext_t() noexcept
            : lut_chars(lambda{})
        {
        }
    };

    static
    constexpr
    qdtext_t qdtext{};

/*
    qpchar = ( HTAB / SP / VCHAR / obs-text )
*/
    static
    constexpr
    struct
    {
        constexpr
        bool
        operator()(char c) const noexcept
        {
            return !(
                c == 127 || (
                    c >= 0 &&
                    c <= 31 &&
                    c != 9));
        }
    } qpchars{};

    auto const start = it;

    // DQUOTE
    if(! grammar::parse(
        it, end, ec, '\"'))
        return;

    // *( qdtext / quoted-pair ) DQUOTE
    for(;;)
    {
        // qdtext
        it = grammar::find_if_not(
            it, end, qdtext);
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return;
        }

        if(*it == '\\')
        {
            // quoted-pair
            ++it;
            if(it == end)
            {
                ec = grammar::error::incomplete;
                return;
            }
            if(! qpchars(*it))
            {
                ec = grammar::error::syntax;
                return;
            }
            ++it;
            continue;
        }

        if(*it == '\"')
        {
            ++it;
            break;
        }

        // expected DQUOTE
        ec = grammar::error::syntax;
        return;
    }

    t.v = string_view(start, it - start);
}

std::string
unquote_text(
    char* const start,
    char const* end)
{
    std::string s;
    s.reserve(end - start);
    // return the same text
    // if it is not quoted
    if( s.size() < 2 ||
        s.front() != '\"' ||
        s.back() != '\"')
    {
        s.append(start,
            end - start);
        return s;
    }
    auto it = start + 1;
    end--;
    while(it != end)
    {
        if(*it != '\\')
        {
            s.push_back(*it);
        }
        else
        {
            ++it;
            if(it == end)
                break;
            s.push_back(*it);
        }
        ++it;
    }
    return s;
}

} // http_proto
} // boost

#endif
