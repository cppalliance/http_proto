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

} // detail
} // http_proto
} // boost

#endif
