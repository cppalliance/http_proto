//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_REQUEST_LINE_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_REQUEST_LINE_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/version.hpp>
#include <boost/url/grammar/parse_tag.hpp>

namespace boost {
namespace http_proto {

/** Rule for request-line

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

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.1.1"
        >3.1.1. Request Line (rfc7230)</a>

    @see
        @ref token.
*/
struct request_line_rule
{
    /** Method
    */
    method m;

    /** Verb
    */
    string_view ms;

    /** Target
    */
    string_view t;

    /** Version
    */
    version v; // 2 digits

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        request_line_rule& t) noexcept
    {
        parse(it, end, ec, t);
    }

private:
    BOOST_HTTP_PROTO_DECL
    static
    void
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        request_line_rule& t) noexcept;
};

} // http_proto
} // boost

#endif
