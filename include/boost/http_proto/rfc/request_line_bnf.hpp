//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_REQUEST_LINE_BNF_HPP
#define BOOST_HTTP_PROTO_RFC_REQUEST_LINE_BNF_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** BNF for request-line

    @par BNF
    @code
    request-line    = method SP request-target SP HTTP-version CRLF
    method          = token
    request-target  = origin-form /
                      absolute-form /
                      authority-form /
                      asterisk-form
    HTTP-version    = "HTTP/" DIGIT "." DIGIT
    @endcode

    @see
        @ref token
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.1.1
*/
struct request_line_bnf
{
    string_view method;
    string_view target;
    char version; // 2 digits

    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
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
