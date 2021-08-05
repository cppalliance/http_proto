//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_TOKEN_LIST_IPP
#define BOOST_HTTP_PROTO_IMPL_TOKEN_LIST_IPP

#include <boost/http_proto/token_list.hpp>
#include <boost/http_proto/ctype.hpp>

namespace boost {
namespace http_proto {

// https://datatracker.ietf.org/doc/html/rfc5234
// https://datatracker.ietf.org/doc/html/rfc7230#section-7
// https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
/*
    token-list        = 1#token
    token             = 1*tchar
    tchar             = "!" / "#" / "$" / "%" / "&" /
                        "'" / "*" / "+" / "-" / "." /
                        "^" / "_" / "`" / "|" / "~" /
                        DIGIT / ALPHA

    legacy list rules:
    1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
*/

char const*
token_list_type::
begin(
    state& st,
    char const* const start,
    char const* const end,
    error_code& ec) noexcept
{
    auto it = start;
   
    // *( "," OWS )
    if(it != end)
    {
        if(*it != ',')
            goto do_token;
        while(++it != end)
        {
            switch(*it)
            {
                case ',':
                case ' ':
                case '\t':
                    ++it;
                    break;
                default:
                    goto do_token;
            }
        }
    }
    // can't have 0 tokens
    ec = error::bad_value;
    return start;

    // token
do_token:
    if(! is_tchar(*it) )
    {
        ec = error::bad_value;
        return start;
    }
    auto const first = it;
    ++it;
    for(;;)
    {
        if( it == end ||
            ! is_tchar(*it))
            break;
        ++it;
    }
    st.value = {first, static_cast<
        std::size_t>(it - first)};
    return it;
}

char const*
token_list_type::
increment(
    state& st,
    char const* const start,
    char const* const end,
    error_code& ec) noexcept
{
    auto it = start;

    // *( OWS "," [ OWS token ] )
    for(;;)
    {
        if(it == end)
            return nullptr;
        switch(*it)
        {
        case ' ':
        case '\t':
        case ',':
            ++it;
            continue;
        default:
            break;
        }
        if(! is_tchar(*it))
        {
            ec = error::bad_value;
            return start;
        }
        break;
    }
    // token
    auto const first = it;
    while(++it != end)
        if(! is_tchar(*it))
            break;
    st.value = {first, static_cast<
        std::size_t>(it - first)};
    return it;
}

} // http_proto
} // boost

#endif
