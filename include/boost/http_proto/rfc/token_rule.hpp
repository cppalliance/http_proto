//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_TOKEN_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_TOKEN_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/url/grammar/lut_chars.hpp>
#include <boost/url/grammar/token_rule.hpp>

namespace boost {
namespace http_proto {

/** The set of token characters

    @par BNF
    @code
    tchar       = "!" / "#" / "$" / "%" / "&" / "'" / "*"
                / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
                / DIGIT / ALPHA
                ; any VCHAR, except delimiters

    VCHAR       =  %x21-7E
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.3"
        >3.2.3. Whitespace (rfc7230)</a>
    @li <a href="https://datatracker.ietf.org/doc/html/rfc5234#appendix-B.1"
        >B.1. Core Rules (rfc5234)</a>
*/
constexpr grammar::lut_chars tchars =
    "!#$%&'*+-.^_`|~"
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

/** Match a token

    @par BNF
    @code
    token = 1*tchar
    @endcode
*/
constexpr auto token_rule = grammar::token_rule( tchars );

} // http_proto
} // boost

#endif
