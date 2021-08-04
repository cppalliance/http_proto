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

#include <boost/http_proto/ctype.hpp>

namespace boost {
namespace http_proto {

namespace detail {

template<std::size_t N>
struct digest_type
{
    // Only 4 and 8 byte is supported
    static_assert(N == 4, "");
    static std::uint32_t const prime = 0x01000193UL;
    static std::uint32_t const hash  = 0x811C9DC5UL;
};

template<>
struct digest_type<8>
{
    static std::uint64_t const prime = 0x100000001B3ULL;
    static std::uint64_t const hash  = 0xcbf29ce484222325ULL;
};

} // detail

std::size_t
idigest(
    string_view s) noexcept
{
    auto p = s.data();
    auto n = s.size();
    using T = detail::digest_type<
        sizeof(std::size_t)>;
    auto const prime = T::prime;
    auto hash = T::hash;
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

} // http_proto
} // boost

#endif

