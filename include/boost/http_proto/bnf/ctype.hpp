//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_CTYPE_HPP
#define BOOST_HTTP_PROTO_CTYPE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/bnf/char_set.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** Return true if c is a decimal digit

    @par BNF
    @code
    DIGIT = "0" / "1" / "2" / "3" / "4" /
            "5" / "6" / "7" / "8" / "9"
    @endcode
*/
inline
bool
is_digit(char c) noexcept;

/** Return true if c is a qpchar

    @par BNF
    @code
    qpchar = ( HTAB / SP / VCHAR / obs-text )
    obs-text    = %x80-FF
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6
        https://datatracker.ietf.org/doc/html/rfc5234#appendix-B.1
*/
inline
bool
is_qpchar(char c) noexcept;

/** Return c converted to lower case
*/
inline
char
to_lower(char c) noexcept;

/** Return a digest of s, using a case-insensitive formula
*/
BOOST_HTTP_PROTO_DECL
std::size_t
idigest(
    string_view s) noexcept;

/** Return true if s1 and s2 are equal, using case-insensitive comparison
*/
BOOST_HTTP_PROTO_DECL
bool
iequals(
    string_view s1,
    string_view s2) noexcept;

/** Return true if s1 is less than s2, using case-insensitive comparison
*/
BOOST_HTTP_PROTO_DECL
bool
iless(
    string_view s1,
    string_view s2) noexcept;

/** Return a digest of s, using a case-insensitive formula
*/
struct ihash
{
    using is_transparent = void;

    std::size_t
    operator()(
        string_view s) const noexcept
    {
        return idigest(s);
    }
};

/** Return true if s1 and s2 are equal, using case-insensitive comparison
*/
struct iequals_pred
{
    using is_transparent = void;

    bool
    operator()(
        string_view s1,
        string_view s2) const noexcept
    {
        return iequals(s1, s2);
    }
};

/** Return true if s1 is less than s2, using case-insensitive comparison
*/
struct iless_pred
{
    using is_transparent = void;

    bool operator()(
        string_view s1,
        string_view s2) const noexcept
    {
        return iless(s1, s2);
    }
};

//------------------------------------------------

/** Character set for tchar

    @par BNF
    @code
    tchar   = "!" / "#" / "$" / "%" / "&" /
              "'" / "*" / "+" / "-" / "." /
              "^" / "_" / "`" / "|" / "~" /
              DIGIT / ALPHA
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#appendix-B
*/
struct tchar_set : char_set_table
{
    BOOST_HTTP_PROTO_DECL
    tchar_set() noexcept;
};

/** Character set for field-vchar

    Valid characters for HTTP field values.

    @par BNF
    @code
    field-vchar = VCHAR / obs-text

    VCHAR       =  %x21-7E
                ; visible (printing) characters

    obs-text    = %x80-FF
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#appendix-B
*/
struct field_vchar_set : char_set_table
{
    BOOST_HTTP_PROTO_DECL
    field_vchar_set() noexcept;
};

/** Character set for qdtext

    @par BNF
    @code
    qdtext      = HTAB / SP /%x21 / %x23-5B / %x5D-7E / obs-text
    obs-text    = %x80-FF
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6
*/
struct qdtext_set : char_set_table
{
    BOOST_HTTP_PROTO_DECL
    qdtext_set() noexcept;
};

/** Character set for qpchar

    @par BNF
    @code
    qpchar = ( HTAB / SP / VCHAR / obs-text )
    obs-text    = %x80-FF
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6
        https://datatracker.ietf.org/doc/html/rfc5234#appendix-B.1
*/
using qpchar_set
    = char_set_function<&is_qpchar>;

} // bnf
} // http_proto
} // boost

#include <boost/http_proto/bnf/impl/ctype.hpp>

#endif
