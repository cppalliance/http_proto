//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_STRING_HPP
#define BOOST_HTTP_PROTO_DETAIL_STRING_HPP

#include <boost/http_proto/string_view.hpp>
#include <cstdint>
#include <string>

namespace boost {
namespace http_proto {
namespace detail {

// Pulling in the UDL directly breaks in some places on MSVC,
// so introduce a namespace for this purprose.
namespace string_literals {
inline
string_view
operator"" _sv(char const* p, std::size_t n) noexcept
{
    return string_view(p, n);
}
} // string_literals

inline
char
ascii_tolower(char c) noexcept
{
    return ((static_cast<unsigned>(c) - 65U) < 26) ?
        c + 'a' - 'A' : c;
}

// Return `true` if two strings are equal,
// using a case-insensitive comparison.
inline
bool
iequals(
    string_view lhs,
    string_view rhs) noexcept
{
    auto n = lhs.size();
    if(rhs.size() != n)
        return false;
    auto p1 = lhs.data();
    auto p2 = rhs.data();
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
        if( ascii_tolower(a) !=
            ascii_tolower(b))
            return false;
        a = *p1++;
        b = *p2++;
    }
    while(n--);
    return true;
}

inline
bool
iless(
    string_view lhs,
    string_view rhs) noexcept
{
    if(lhs.size() < rhs.size())
        return true;
    if(lhs.size() > rhs.size())
        return false;
    auto p1 = lhs.data();
    auto p2 = rhs.data();
    for(auto n = lhs.size();n--;)
    {
        auto c1 = ascii_tolower(*p1++);
        auto c2 = ascii_tolower(*p2++);
        if(c1 == c2)
            continue;
        return c1 < c2;
    }
    return false;
}

// Return a case-insensitive digest of the string
inline
std::size_t
idigest(
    char const* s,
    std::size_t n) noexcept
{
    // VFALCO we aren't setting this?
    // Can this be done without a macro, e.g. std::conditional?
#if BOOST_HTTP_PROTO_ARCH == 64
    std::uint64_t const prime = 0x100000001B3ULL;
    std::uint64_t hash  = 0xcbf29ce484222325ULL;
#else
    std::uint32_t const prime = 0x01000193UL;
    std::uint32_t hash  = 0x811C9DC5UL;
#endif
    for(;n--;++s)
        hash = (ascii_tolower(*s) ^ hash) * prime;
    return hash;
}

// Case-insensitive equals function object
struct iequals_pred
{
    using is_transparent = void;

    bool operator()(
        string_view s1,
        std::string const& s2) const noexcept
    {
        return iequals(s1, string_view(
            s2.data(), s2.size()));
    }

    bool operator()(
        std::string const& s1,
        string_view s2) const noexcept
    {
        return iequals(string_view(
            s1.data(), s1.size()), s2);
    }

    bool operator()(
        std::string const& s1,
        std::string const& s2) const noexcept
    {
        return iequals(
            string_view(s1.data(), s1.size()),
            string_view(s2.data(), s2.size()));
    }
};

// Case-insensitive less function object
struct iless_pred
{
    using is_transparent = void;

    bool operator()(
        string_view s1,
        std::string const& s2) const noexcept
    {
        return iless(s1, string_view(
            s2.data(), s2.size()));
    }

    bool operator()(
        std::string const& s1,
        string_view s2) const noexcept
    {
        return iless(string_view(
            s1.data(), s1.size()), s2);
    }

    bool operator()(
        std::string const& s1,
        std::string const& s2) const noexcept
    {
        return iless(
            string_view(s1.data(), s1.size()),
            string_view(s2.data(), s2.size()));
    }
};

// Case-insensitive string hash function
struct ihash
{
    using is_transparent = void;

    std::size_t operator()(
        string_view s) const noexcept
    {
        return idigest(s.data(), s.size());
    }

    std::size_t operator()(
        std::string const& s) const noexcept
    {
        return idigest(s.data(), s.size());
    }
};

} // detail
} // http_proto
} // boost

#endif
