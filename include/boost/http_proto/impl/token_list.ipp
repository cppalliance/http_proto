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
// https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
// https://datatracker.ietf.org/doc/html/rfc7230#section-7
/*
    token-list        = 1#token
    token             = 1*tchar

    legacy list rules:
    1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )

    To satisfy the requirement for greedy parsing,
    in begin we consume this bnf:

        *( "," OWS ) element *( OWS "," )

    and in increment we consume:

        element *( OWS "," )

    with an additional flag to indicate a comma was seen.
*/

char const*
token_list_bnf::
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
token_list_bnf::
skip_opt_ows_comma(
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
                comma_ =
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
            comma_ =
                last != start;
            return last;
        }
    }
}

char const*
token_list_bnf::
skip_token(
    char const* it,
    char const* const end) noexcept
{
    while(it != end)
    {
        if(! is_tchar(*it))
            break;
        ++it;
    }
    return it;
}

char const*
token_list_bnf::
begin(
    char const* const start,
    char const* const end,
    error_code& ec) noexcept
{
//  *( "," OWS ) token *( OWS "," )
    auto first = skip_opt_comma_ows(
        start, end);
    auto it = skip_token(first, end);
    if(it == first)
    {
        // missing token
        ec = error::bad_value;
        return start;
    }
    value = { first, static_cast<
        std::size_t>(it - first) };
    return skip_opt_ows_comma(
        it, end);
}

char const*
token_list_bnf::
increment(
    char const* const start,
    char const* const end,
    error_code& ec) noexcept
{
//  element *( OWS "," )
    if(start == end)
        return nullptr;
    if(! comma_)
    {
        ec = error::bad_value;
        return start;
    }
    auto it = skip_token(
        start, end);
    if(it == start)
    {
        ec = error::bad_value;
        return start;
    }
    value = { start, static_cast<
        std::size_t>(it - start) };
    return skip_opt_ows_comma(
        it, end);
}

} // http_proto
} // boost

#endif
