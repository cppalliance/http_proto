//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_FIELD_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_FIELD_RULE_IPP

#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/http_proto/rfc/ows_rule.hpp>
#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/lut_chars.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/token_rule.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

bool
parse(
    char const*& it,
    char const* end,
    error_code& ec,
    field_rule& t) noexcept
{
    using grammar::parse;

    struct field_chars_t
    {
        constexpr
        bool
        operator()(char c) const noexcept
        {
            return ((static_cast<unsigned char>(
                    c) >= 32) && (c != 127))
                || (c == 9);
        }
    };

    auto const start = it;

    if(it == end)
    {
        ec = grammar::error::incomplete;
        return false;
    }

    // check for leading CRLF
    if(it[0] == '\r')
    {
        ++it;
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return false;
        }
        if(*it != '\n')
        {
            ec = grammar::error::syntax;
            return false;
        }
        // end of fields
        ++it;
        ec = grammar::error::end;
        return false;
    }

    // field name
    grammar::token_rule<tchars_t> t0;
    if(! parse(it, end, ec, t0, ':'))
        return false;
    t.v.name = string_view(start, it - 1);
    t.v.has_obs_fold = false;

    // consume all obs-fold until
    // field char or end of field
    for(;;)
    {
        if(! parse(it, end, ec, ows_rule{}))
            return false;
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return false;
        }
        if(*it != '\r')
        {
            // start of value
            break;
        }
        ++it;
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return false;
        }
        if(*it != '\n')
        {
            ec = grammar::error::syntax;
            return false;
        }
        ++it;
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return false;
        }
        if(*it == '\r')
        {
            // empty value
            t.v.value = {};
            return true;
        }
        if( *it != ' ' &&
            *it != '\t')
        {
            // start of value
            break;
        }
        // eat obs-fold
        ++it;
        t.v.has_obs_fold = true;
    }

    char const* s0 = it; // start of value

    grammar::token_rule<field_chars_t> t1;

    for(;;)
    {
        if(! parse(it, end, ec, t1))
            return false;
        if(! parse(it, end, ec, '\r', '\n'))
            return false;
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return false;
        }
        if( *it != ' ' &&
            *it != '\t')
        {
            // end of field
            break;
        }
        // t1 will parse *it
        t.v.has_obs_fold = true;
    }

    t.v.value = string_view(s0, (it - s0) - 2);
    BOOST_ASSERT(! t.v.value.empty());
    BOOST_ASSERT(! ws(t.v.value.front()));

    // remove trailing SP,HTAB,CR,LF
    auto p = &t.v.value.back();
    for(;;)
    {
        switch(*p)
        {
        case ' ':  case '\t':
        case '\r': case '\n':
            --p;
            continue;
        default:
            ++p;
            goto done;
        }
    }
done:
    t.v.value = string_view(
        t.v.value.data(),
        p - t.v.value.data());
    return true;
}

//------------------------------------------------

void
replace_obs_fold(
    char* it,
    char const* const end) noexcept
{
    while(it != end)
    {
        if(*it != '\r')
        {
            ++it;
            continue;
        }
        if(end - it < 3)
            break;
        BOOST_ASSERT(it[1] == '\n');
        if( it[1] == '\n' &&
            ws(it[2]))
        {
            it[0] = ' ';
            it[1] = ' ';
            it += 3;
        }
        else
        {
            ++it;
        }
    }
}

} // http_proto
} // boost

#endif
