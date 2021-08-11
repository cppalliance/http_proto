//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_REQUIRED_LIST_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_REQUIRED_LIST_IPP

#include <boost/http_proto/bnf/required_list.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
required_list::
begin(
    char const* start,
    char const* end,
    error_code& ec)
{
    // *( "," OWS )
    auto const first =
        detail::skip_opt_comma_ows(
            start, end);
    // element
    auto it = parse_element(
        first, end, ec);
    if(ec)
        return it;
    BOOST_ASSERT(it != first);
    // *( OWS "," )
    return detail::skip_opt_ows_comma(
        comma_, it, end);
}

char const*
required_list::
increment(
    char const* start,
    char const* end,
    error_code& ec)
{
    ws_set ws;
    // [ ... ]
    if(start == end)
    {
        ec = error::end;
        return end;
    }
    if(! comma_)
    {
        // invalid character
        ec = error::bad_list;
        return start;
    }
    // OWS
    auto const first =
        ws.skip(start, end);
    // element
    auto it = parse_element(
        first, end, ec);
    if(ec)
        return it;
    BOOST_ASSERT(it != first);
    // *( OWS "," )
    return detail::skip_opt_ows_comma(
        comma_, it, end);
}

} // bnf
} // http_proto
} // boost

#endif
