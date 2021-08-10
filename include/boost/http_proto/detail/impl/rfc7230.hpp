//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_RFC7230_HPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_RFC7230_HPP

namespace boost {
namespace http_proto {
namespace detail {

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
