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
#include <boost/http_proto/bnf/detail/optional_list_base.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

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
class optional_list
    : private detail::optional_list_base
{
    Element element_;

    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec) override
    {
        return element_.parse(
            start, end, ec);
    }

public:
    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec)
    {
        return optional_list_base::begin(
            start, end, ec);
    }

    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec)
    {
        return optional_list_base::increment(
            start, end, ec);
    }
};

} // bnf
} // http_proto
} // boost

#endif
