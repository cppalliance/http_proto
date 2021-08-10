//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_REQUIRED_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_REQUIRED_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/rfc7230.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

/** Mix-in for 1#element BNF

    @par BNF
    @code
    1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
    @endcode

    @par Requirements
    Derived must have this member function
    @code
        char const*
        parse_element(
            char const* start,
            char const* end,
            error_code& ec);
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-7
*/
template<class Derived>
struct required_list
{
    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec);

    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec);

private:
    Derived&
    self() noexcept
    {
        return *static_cast<
            Derived*>(this);
    }

    bool comma_;
};

template<class Derived>
char const*
required_list<Derived>::
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
    auto it = self().parse_element(
        first, end, ec);
    if(ec)
        return it;
    BOOST_ASSERT(it != first);
    // *( OWS "," )
    return detail::skip_opt_ows_comma(
        comma_, it, end);
}

template<class Derived>
char const*
required_list<Derived>::
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
    auto it = self().parse_element(
        first, end, ec);
    if(ec)
        return it;
    BOOST_ASSERT(it != first);
    // *( OWS "," )
    return detail::skip_opt_ows_comma(
        comma_, it, end);
}

} // http_proto
} // boost

#endif
