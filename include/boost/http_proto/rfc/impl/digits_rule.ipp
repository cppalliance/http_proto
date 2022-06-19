//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_DIGITS_RULE_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_DIGITS_RULE_IPP

#include <boost/http_proto/rfc/digits_rule.hpp>
#include <boost/url/grammar/charset.hpp>
#include <limits>

namespace boost {
namespace http_proto {

void
digits_rule::
parse(
    char const*& it,
    char const* end,
    error_code& ec,
    digits_rule& t) noexcept
{
    if(it == end)
    {
        ec = grammar::error::syntax;
        return;
    }

    auto const it0 = it;
    t.overflow = false;

    // discard leading zeroes
    // otherwise safe_end is wrong
    if(*it == '0')
    {
        do
        {
            ++it;
            if(it == end)
            {
                t.s = string_view(it0, it);
                t.v = 0;
                return;
            }
        }
        while(*it == '0');

        if(! grammar::digit_chars(*it))
        {
            t.s = string_view(it0, it);
            t.v = 0;
            return;
        }
    }
    else if(! grammar::digit_chars(*it))
    {
        ec = error::syntax;
        return;
    }

    using T = std::uint64_t;

    // first char
    static constexpr T D =
        std::numeric_limits<
            T>::digits10;
    static constexpr T ten = 10;
    auto const safe_end = (
        std::min)(it + D, end);
    T v = *it - '0';
    ++it;

    // chars that can't overflow
    while(it != safe_end &&
        grammar::digit_chars(*it))
    {
        char const dig = *it - '0';
        v = v * ten + dig;
        ++it;
    }

    if( it != end &&
        grammar::digit_chars(*it))
    {
        // one more digit could overflow
        static constexpr T Max = (
            std::numeric_limits<
                T>::max)();
        static constexpr auto div = (Max / ten);
        static constexpr char rem = (Max % ten);
        char const dig = *it - '0';
        if( v < div || (
            v == div && dig <= rem))
        {
            v = v * ten + dig;
        }
        else
        {
            t.overflow = true;
        }
        ++it;

        // eat remaining digits
        auto const it1 =
            grammar::find_if_not(it, end,
                grammar::digit_chars);
        if(it1 != it)
        {
            // definite overflow
            it = it1;
            t.overflow = true;
        }
    }

    t.v = v;
    t.s = string_view(it0, it);
}

} // http_proto
} // boost

#endif
