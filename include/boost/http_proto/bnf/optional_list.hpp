//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_OPTIONAL_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_OPTIONAL_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** BNF for a comma-separated list with zero or more elements

    @par BNF
    @code
    #element => [ ( "," / element ) *( OWS "," [ OWS element ] ) ]
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-7
*/
template<class Element>
struct optional_list
{
    using value_type =
        typename Element::value_type;

    value_type value;

    BOOST_HTTP_PROTO_DECL
    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec);

protected:
    virtual
    char const*
    parse_element(
        char const* start,
        char const* end,
        error_code& ec) = 0;

private:
    bool comma_;
};

} // http_proto
} // boost

#endif
