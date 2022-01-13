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
#include <boost/url/grammar/lut_chars.hpp>

namespace boost {
namespace http_proto {

/** The horizontal whitespace character set type.

    Objects of this type are invocable
    with this equivalent signature:

    @code
    bool( char ch ) const noexcept;
    @endcode

    The function object returns `true` when
    `ch` is a member of the character set,
    and `false` otherwise. This type satisfies
    the <em>CharSet</em> requirements.

    @par BNF
    @code
    WS      = SP / HTAB
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.3"
        >3.2.3.  Whitespace (rfc7230)</a>

    @see
        @ref ws.
*/
struct ws_t
{
    constexpr
    bool
    operator()(char c) const noexcept
    {
        return c == ' ' || c == '\t';
    }
};

/** The horizontal whitespace character set type.

    Objects of this type are invocable
    with this equivalent signature:

    @code
    bool( char ch ) const noexcept;
    @endcode

    The function object returns `true` when
    `ch` is a member of the character set,
    and `false` otherwise. This type satisfies
    the <em>CharSet</em> requirements.

    @par BNF
    @code
    WS      = SP / HTAB
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.3"
        >3.2.3.  Whitespace (rfc7230)</a>

    @see
        @ref ws.
*/
constexpr ws_t ws{};

//------------------------------------------------

/** The pchar character set type.

    Objects of this type are invocable
    with this equivalent signature:

    @code
    bool( char ch ) const noexcept;
    @endcode

    The function object returns `true` when
    `ch` is a member of the character set,
    and `false` otherwise. This type satisfies
    the <em>CharSet</em> requirements.

    @par BNF
    @code
    pchar       = <any OCTET except CTLs, and excluding LWS>
    @endcode

    @see
        @ref pchars.
*/
struct pchars_t
{
    constexpr
    bool
    operator()(char c) const noexcept
    {
        return 
            (static_cast<unsigned char>(c) > 32) &&
            (c != 127);
    }
};

/** The pchar character set type.

    Objects of this type are invocable
    with this equivalent signature:

    @code
    bool( char ch ) const noexcept;
    @endcode

    The function object returns `true` when
    `ch` is a member of the character set,
    and `false` otherwise. This type satisfies
    the <em>CharSet</em> requirements.

    @par BNF
    @code
    pchar       = <any OCTET except CTLs, and excluding LWS>
    @endcode

    @see
        @ref pchars_t.
*/
constexpr pchars_t pchars{};

//------------------------------------------------

/** The token character set type.

    Objects of this type are invocable
    with this equivalent signature:

    @code
    bool( char ch ) const noexcept;
    @endcode

    The function object returns `true` when
    `ch` is a member of the character set,
    and `false` otherwise. This type satisfies
    the <em>CharSet</em> requirements.

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

    @see
        @ref tchars
*/
struct tchars_t
    : grammar::lut_chars
{
    constexpr
    tchars_t() noexcept
        : lut_chars(
            "!#$%&'*+-.^_`|~"
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
        )
    {
    }
};

/** The token character set type.

    Objects of this type are invocable
    with this equivalent signature:

    @code
    bool( char ch ) const noexcept;
    @endcode

    The function object returns `true` when
    `ch` is a member of the character set,
    and `false` otherwise. This type satisfies
    the <em>CharSet</em> requirements.

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

    @see
        @ref tchars_t
*/
constexpr tchars_t tchars{};

} // http_proto
} // boost

#endif
