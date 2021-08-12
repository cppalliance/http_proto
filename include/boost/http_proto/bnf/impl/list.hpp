//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_IMPL_LIST_HPP

#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/sequence.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

template<
    class Element,
    std::size_t N,
    std::size_t M>
char const*
list<Element, N, M>::
begin(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    n_ = 0;
    // empty
    if(start == end)
    {
        if(n_ < N)
        {
            // too few
            ec = error::syntax;
            return start;
        }
        ec = error::end;
        return start;
    }
    // ( element ) ;most common
    auto it = element_.parse(
        start, end, ec);
    if(! ec)
    {
        ++n_;
        return it;
    }
    ec = {};
    // *( "," OWS ) element
    it = consume<zero_or_more<
        comma_ows>>(it, end, ec);
    BOOST_ASSERT(! ec);
    it = element_.parse(
        it, end, ec);
    if(! ec)
    {
        ++n_;
        return it;
    }
    // empty
    if(n_ < N)
    {
        // too few
        ec = error::syntax;
        return start;
    }
    ec = error::end;
    return start;
}

template<
    class Element,
    std::size_t N,
    std::size_t M>
char const*
list<Element, N, M>::
increment(
    char const* start,
    char const* const end,
    error_code& ec)
{
    ws_set ws;

    if(n_ >= M)
    {
        // too many
        ec = error::end;
        return start;
    }
    // *( "," OWS )
    auto it = consume<
        zero_or_more<ows_comma>>(
            start, end, ec);
    BOOST_ASSERT(! ec);
    if(it == start)
    {
        // no comma
        if(n_ < N)
        {
            // too few
            ec = error::syntax;
            return start;
        }
        ec = error::end;
        return start;
    }
    start = it;
    // [ OWS element ]
    it = ws.skip(it, end);
    it = element_.parse(
        it, end, ec);
    if(ec)
    {
        if(n_ < N)
        {
            // too few
            ec = error::syntax;
            return start;
        }
        ec = error::end;
        return start;
    }
    ++n_;
    return it;
}

//------------------------------------------------

char const*
ows_comma::
parse(
    char const* start,
    char const* end,
    error_code& ec)
{
    ws_set ws;
    auto it = ws.skip(
        start, end);
    if(it == end)
    {
        // missing comma
        ec = error::syntax;
        return start;
    }
    if(*it != ',')
    {
        // expected comma
        ec = error::syntax;
        return start;
    }
    ++it;
    return it;
}

char const*
comma_ows::
parse(
    char const* start,
    char const* end,
    error_code& ec)
{
    ws_set ws;
    if(start == end)
    {
        ec = error::syntax;
        return start;
    }
    auto it = start;
    if(*it != ',')
    {
        ec = error::syntax;
        return it;
    }
    ++it;
    it = ws.skip(it, end);
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
