//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_CHARSETS_HPP
#define BOOST_HTTP_PROTO_RFC_CHARSETS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/url/bnf/lut_chars.hpp>

namespace boost {
namespace http_proto {

/** Character set for WS

    @par BNF
    @code
    WS      = SP / HTAB
    @endcode
*/
constexpr urls::bnf::lut_chars ws = " \t";

struct tchars_type
    : urls::bnf::lut_chars
{
    constexpr
    tchars_type() noexcept
        : lut_chars(
            "!#$%&'*+-.^_`|~"
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
        )
    {
    }
};

/** Character set for tchar

    @par BNF
    @code
    tchar   = "!" / "#" / "$" / "%" / "&"
            / "'" / "*" / "+" / "-" / "."
            / "^" / "_" / "`" / "|" / "~"
            / DIGIT / ALPHA
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6"
        >3.2.6. Field Value Components (rfc7230)</a>
*/
constexpr tchars_type tchars;

} // http_proto
} // boost

#endif
