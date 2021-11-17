//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_QUOTED_STRING_HPP
#define BOOST_HTTP_PROTO_BNF_QUOTED_STRING_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <string>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for quoted-string

    @par BNF
    @code
    quoted-string   = DQUOTE *( qdtext / quoted-pair ) DQUOTE
    qdtext          = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
    obs-text        = %x80-FF
    quoted-pair     = "\" ( HTAB / SP / VCHAR / obs-text )
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6
*/
class quoted_string
{
public:
    using value_type = string_view;

    value_type const&
    value() const noexcept
    {
        return v_;
    }

    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec);

private:
    value_type v_;
};

BOOST_HTTP_PROTO_DECL
std::string
unquote_text(
    char* start,
    char const* end);

} // bnf
} // http_proto
} // boost

#endif
