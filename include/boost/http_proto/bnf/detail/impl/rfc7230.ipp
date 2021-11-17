//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_DETAIL_IMPL_RFC7230_IPP
#define BOOST_HTTP_PROTO_BNF_DETAIL_IMPL_RFC7230_IPP

#include <boost/http_proto/bnf/detail/rfc7230.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/ctype.hpp>

namespace boost {
namespace http_proto {
namespace bnf {
namespace detail {

static
bool
is_pchar(char c) noexcept
{
    // TEXT = <any OCTET except CTLs, and excluding LWS>
    static bool constexpr tab[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //   0
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  16
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  32
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  48
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  64
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  80
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  96
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 112
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 128
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 144
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 160
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 176
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 192
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 208
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 224
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  // 240
    };
    return tab[static_cast<unsigned char>(c)];
}

char const*
parse_http_version(
    char& result,
    char const* const start,
    char const* const end,
    error_code& ec) noexcept
{
    // HTTP-version = "HTTP/" DIGIT "." DIGIT
    if(start == end)
    {
        ec = error::need_more;
        return start;
    }
    auto it = start;
    auto n = end - it;
    if( n > 7)
        n = 7;
    if(std::memcmp(it,
        "HTTP/1.", n) != 0)
    {
        // fail fast
        ec = error::bad_version;
        return start;
    }
    it += n;
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it == '0')
    {
        result = 10;
    }
    else if(*it == '1')
    {
        result = 11;
    }
    else
    {
        ec = error::bad_version;
        return start;
    }
    ++it;
    return it;
}

} // detail
} // bnf
} // http_proto
} // boost

#endif
