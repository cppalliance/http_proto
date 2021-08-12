//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CTYPE_IPP
#define BOOST_HTTP_PROTO_IMPL_CTYPE_IPP

#include <boost/http_proto/bnf/ctype.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

std::size_t
idigest(
    string_view s) noexcept
{
    // Only 4 and 8 byte sizes are supported
    static_assert(
        sizeof(std::size_t) == 4 ||
        sizeof(std::size_t) == 8, "");
    constexpr std::size_t prime = (
        sizeof(std::size_t) == 8) ?
            0x100000001B3ULL :
            0x01000193UL;
    constexpr std::size_t hash0 = (
        sizeof(std::size_t) == 8) ?
            0xcbf29ce484222325ULL :
            0x811C9DC5UL;
    auto hash = hash0;
    auto p = s.data();
    auto n = s.size();
    for(;n--;++p)
    {
        // VFALCO NOTE Consider using a lossy
        // to_lower which works 4 or 8 chars at a time.
        hash = (to_lower(*p) ^ hash) * prime;
    }
    return hash;
}

// https://lemire.me/blog/2020/04/30/for-case-insensitive-string-comparisons-avoid-char-by-char-functions/
// https://github.com/lemire/Code-used-on-Daniel-Lemire-s-blog/blob/master/2020/04/30/tolower.cpp
bool
iequals(
    string_view s1,
    string_view s2) noexcept
{
    auto n = s1.size();
    if(s2.size() != n)
        return false;
    auto p1 = s1.data();
    auto p2 = s2.data();
    char a, b;
    // fast loop
    while(n--)
    {
        a = *p1++;
        b = *p2++;
        if(a != b)
            goto slow;
    }
    return true;
slow:
    do
    {
        if( to_lower(a) !=
            to_lower(b))
            return false;
        a = *p1++;
        b = *p2++;
    }
    while(n--);
    return true;
}

bool
iless(
    string_view s1,
    string_view s2) noexcept
{
    if(s1.size() < s2.size())
        return true;
    if(s1.size() > s2.size())
        return false;
    auto p1 = s1.data();
    auto p2 = s2.data();
    for(auto n = s1.size();n--;)
    {
        auto c1 = to_lower(*p1++);
        auto c2 = to_lower(*p2++);
        if(c1 == c2)
            continue;
        return c1 < c2;
    }
    return false;
}

//------------------------------------------------

hexdig_set::
hexdig_set() noexcept
    : char_set_table(
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
"\0\1\1\1\1\1\1\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\1\1\1\1\1\1\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    )
{
}

tchar_set::
tchar_set() noexcept
    : char_set_table(
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\1\0\1\1\1\1\1\0\0\1\1\0\1\1\0" "\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
"\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    )
{
}

field_vchar_set::
field_vchar_set() noexcept
    : char_set_table(
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
    )
{
}

qdtext_set::
qdtext_set() noexcept
    : char_set_table(
"\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\1\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\0\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
"\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
    )
{
}

} // bnf
} // http_proto
} // boost

#endif

