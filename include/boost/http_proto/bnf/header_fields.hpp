//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_HEADER_FIELDS_HPP
#define BOOST_HTTP_PROTO_BNF_HEADER_FIELDS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/bnf/range.hpp>

namespace boost {
namespace http_proto {

/** BNF for header-fields

    @par BNF
    @code
    header-field   = field-name ":" OWS field-value OWS

    field-name      = token
    field-value     = *( field-content / obs-fold )
    field-content   = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]

    obs-fold        = OWS CRLF 1*( SP / HTAB )
                    ; obsolete line folding
                    ; see Section 3.2.4

    token           = 1*tchar
    @endcode

    @see
        https://www.rfc-editor.org/errata/eid4189
        https://datatracker.ietf.org/doc/html/rfc7230#appendix-B
*/
struct header_fields_bnf
{
    struct value_type
    {
        string_view name;
        string_view value;
        bool has_obs_fold;
    };

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
};

using header_fields = range<header_fields_bnf>;

BOOST_HTTP_PROTO_DECL
void
replace_obs_fold(
    char *start,
    char const* end) noexcept;

} // http_proto
} // boost

#endif
