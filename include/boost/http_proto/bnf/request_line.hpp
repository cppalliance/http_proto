//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_REQUEST_LINE_HPP
#define BOOST_HTTP_PROTO_BNF_REQUEST_LINE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** BNF for request-line

    @par BNF
    @code
    transfer-param-list = *( OWS ";" OWS transfer-param )
    transfer-param      = token BWS "=" BWS ( token / quoted-string )

    quoted-string       = DQUOTE *( qdtext / quoted-pair ) DQUOTE
    qdtext              = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
    obs-text            = %x80-FF
    @endcode

    @see
*/
struct request_line
{
    struct value_type
    {
        string_view method;
        string_view target;
        int version;
    };

    value_type value;

    BOOST_HTTP_PROTO_DECL
    char const*
    parse_element(
        char const* start,
        char const* end,
        error_code& ec);

private:
    char const*
    parse_method(
        char const* start,
        char const* end,
        error_code& ec);

    char const*
    parse_target(
        char const* start,
        char const* end,
        error_code& ec);
};

} // http_proto
} // boost

#endif
