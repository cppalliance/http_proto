//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_CTYPE_HPP
#define BOOST_HTTP_PROTO_CTYPE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** Return true if c is a digit
*/
inline
bool
is_digit(char c) noexcept;

/** Return true if c is a printable character
*/
inline
bool
is_print(char c) noexcept;

/** Return c converted to lower case
*/
inline
char
to_lower(char c) noexcept;

/** Return a digest of s, using a case-insensitive formula
*/
BOOST_HTTP_PROTO_DECL
std::size_t
idigest(
    string_view s) noexcept;

/** Return true if s1 and s2 are equal, using case-insensitive comparison
*/
BOOST_HTTP_PROTO_DECL
bool
iequals(
    string_view s1,
    string_view s2) noexcept;

/** Return true if s1 is less than s2, using case-insensitive comparison
*/
BOOST_HTTP_PROTO_DECL
bool
iless(
    string_view s1,
    string_view s2) noexcept;

/** Return a digest of s, using a case-insensitive formula
*/
struct ihash
{
    using is_transparent = void;

    std::size_t
    operator()(
        string_view s) const noexcept
    {
        return idigest(s);
    }
};

/** Return true if s1 and s2 are equal, using case-insensitive comparison
*/
struct iequals_pred
{
    using is_transparent = void;

    bool
    operator()(
        string_view s1,
        string_view s2) const noexcept
    {
        return iequals(s1, s2);
    }
};

/** Return true if s1 is less than s2, using case-insensitive comparison
*/
struct iless_pred
{
    using is_transparent = void;

    bool operator()(
        string_view s1,
        string_view s2) const noexcept
    {
        return iless(s1, s2);
    }
};

} // http_proto
} // boost

#include <boost/http_proto/impl/ctype.hpp>

#endif
