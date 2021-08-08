//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_RFC7230_IPP
#define BOOST_HTTP_PROTO_DETAIL_RFC7230_IPP

#include <boost/http_proto/detail/rfc7230.hpp>
#include <boost/http_proto/ctype.hpp>

namespace boost {
namespace http_proto {
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
skip_opt_comma_ows(
    char const* it,
    char const* const end) noexcept
{
    // *( "," OWS )
    if(it == end)
        return it;
    if(*it != ',')
        return it;
    ++it;
    while(it != end)
    {
        switch(*it)
        {
        case ' ':
        case '\t':
        case ',':
            ++it;
            continue;
        }
        break;
    }
    return it;
}

char const*
skip_opt_ows_comma(
    bool& comma,
    char const* const start,
    char const* const end) noexcept
{
    // *( OWS "," )
    auto it = start;
    auto last = start;
    if(it == end)
        return it;
    for(;;)
    {
        switch(*it)
        {
        case ' ':
        case '\t':
            ++it;
            if(it == end)
            {
                comma =
                    last != start;
                return last;
            }
            break;
        case ',':
            ++it;
            if(it == end)
                return it;
            last = it;
            break;
        default:
            comma =
                last != start;
            return last;
        }
    }
}

} // detail
} // http_proto
} // boost

#endif
