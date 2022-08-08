//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_DETAIL_IMPL_RULES_IPP
#define BOOST_HTTP_PROTO_RFC_DETAIL_IMPL_RULES_IPP

#include <boost/http_proto/rfc/detail/rules.hpp>
#include <boost/url/grammar/digit_chars.hpp>

namespace boost {
namespace http_proto {
namespace detail {

auto
crlf_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        result<value_type>
{
    switch(end - it)
    {
    case 0:
    {
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    case 1:
    {
        if(it[0] == '\r')
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::mismatch);
    }
    default:
        break;
    }
    if( it[0] == '\r' &&
        it[1] == '\n')
    {
        it += 2;
        return {};
    }
    BOOST_HTTP_PROTO_RETURN_EC(
        grammar::error::mismatch);
}

//------------------------------------------------

auto
version_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        result<value_type>
{
    value_type v = 0;
    if(it == end)
    {
        // expected "HTTP/"
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    if(end - it >= 5)
    {
        if(std::memcmp(
            it, "HTTP/", 5) != 0)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::mismatch);
        }
        it += 5;
    }
    if(it == end)
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    if(! grammar::digit_chars(*it))
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    v = 10 * (*it++ - '0');
    if(it == end)
    {
        // expected "."
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    if(*it != '.')
    {
        // expected "."
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    ++it;
    if(it == end)
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    if(! grammar::digit_chars(*it))
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    v += *it++ - '0';
    return v;
}

//------------------------------------------------

auto
status_code_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        result<value_type>
{
    auto const dig =
        [](char c) -> int
        {
            unsigned char uc(c - '0');
            if(uc > 9)
                return -1;
            return uc;
        };
        
    if(it == end)
    {
        // end
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    auto it0 = it;
    int v = dig(*it);
    if(v == -1)
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::mismatch);
    }
    value_type t;
    t.v = 100 * v;
    ++it;
    if(it == end)
    {
        // end
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    v = dig(*it);
    if(v == -1)
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::mismatch);
    }
    t.v = t.v + (10 * v);
    ++it;
    if(it == end)
    {
        // end
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    v = dig(*it);
    if(v == -1)
    {
        // expected DIGIT
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    t.v = t.v + v;
    ++it;

    t.s = string_view(it0, it - it0);
    t.st = int_to_status(t.v);
    return t;
}

//------------------------------------------------

auto
field_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        result<value_type>
{
    if(it == end)
    {
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::need_more);
    }
    // check for leading CRLF
    if(it[0] == '\r')
    {
        ++it;
        if(it == end)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if(*it != '\n')
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::mismatch);
        }
        // end of fields
        ++it;
        BOOST_HTTP_PROTO_RETURN_EC(
            grammar::error::range_end);
    }

    value_type v;

    // field name
    {
        auto rv = grammar::parse(
            it, end, grammar::tuple_rule(
                token_rule,
                grammar::squelch(
                    grammar::delim_rule(':'))));
        if(! rv)
            return rv.error();
        v.name = rv.value();
    }

    // consume all obs-fold until
    // field char or end of field
    for(;;)
    {
        skip_ows(it, end);
        if(it == end)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if(*it != '\r')
        {
            // start of value
            break;
        }
        ++it;
        if(it == end)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if(*it != '\n')
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::mismatch);
        }
        ++it;
        if(it == end)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if(*it == '\r')
        {
            // empty value
            return v;
        }
        if( *it != ' ' &&
            *it != '\t')
        {
            // start of value
            break;
        }
        // eat obs-fold
        ++it;
        v.has_obs_fold = true;
    }

    char const* s0 = it; // start of value
    for(;;)
    {
        auto rv = grammar::parse(
            it, end, grammar::tuple_rule(
                grammar::token_rule(
                    ws_vchars),
                crlf_rule));
        if(! rv)
            return rv.error();
        if(it == end)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                grammar::error::need_more);
        }
        if( *it != ' ' &&
            *it != '\t')
        {
            // end of field
            break;
        }
        // *it will match field_value_rule
        v.has_obs_fold = true;
    }

    v.value = string_view(s0, (it - s0) - 2);
    BOOST_ASSERT(! v.value.empty());
    //BOOST_ASSERT(! ws(t.v.value.front()));

    // remove trailing SP,HTAB,CR,LF
    auto p = &v.value.back();
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
    v.value = string_view(
        v.value.data(),
        p - v.value.data());
    return v;
}

//------------------------------------------------

void
remove_obs_fold(
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

} // detail
} // http_proto
} // boost

#endif
