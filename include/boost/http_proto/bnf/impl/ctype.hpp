//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CTYPE_HPP
#define BOOST_HTTP_PROTO_IMPL_CTYPE_HPP

namespace boost {
namespace http_proto {
namespace bnf {

bool
is_digit(char c) noexcept
{
    return static_cast<
        unsigned char>(c-'0') < 10;
}

bool
is_ws(char c) noexcept
{
    return c == ' ' || c == '\t';
}

bool
is_qpchar(char c) noexcept
{
    unsigned char u = c;
    if(u >= 0x20)
        return u != 0x7f;
    return u == 0x09; // HTAB
}

char
to_lower(char c) noexcept
{
    if ((static_cast<unsigned>(
            c) - 65U) < 26)
        return c + ('a' - 'A');
    return c;
}


} // bnf
} // http_proto
} // boost

#endif

