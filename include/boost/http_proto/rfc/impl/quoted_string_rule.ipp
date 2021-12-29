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

namespace detail {

struct qpchars_t
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
};

static constexpr qpchars_t qpchars{};

} // detail

bool
parse(
    char const*& it,
    char const* const end,
    error_code& ec,
    quoted_string_rule& t) noexcept
{
    using grammar::parse;

    auto const start = it;

    // DQUOTE
    if(! parse(it, end, ec, '\"'))
        return false;

    // *( qdtext / quoted-pair ) DQUOTE
    for(;;)
    {
        // qdtext
        it = grammar::find_if_not(
            it, end, qdtext_chars);
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return false;
        }

        if(*it == '\\')
        {
            // quoted-pair
            ++it;
            if(it == end)
            {
                ec = grammar::error::incomplete;
                return false;
            }
            if(! detail::qpchars(*it))
            {
                ec = grammar::error::syntax;
                return false;
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
        return false;
    }

    t.v = string_view(start, it - start);
    return true;
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
