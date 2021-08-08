//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_TRANSFER_PARAMETER_LIST_HPP
#define BOOST_HTTP_PROTO_TRANSFER_PARAMETER_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/string_view.hpp>
#include <utility>

namespace boost {
namespace http_proto {

/** BNF for transfer-param-list

    Parameters are in the form of a name=value pair.

    @par BNF
    @code
    transfer-param-list = *( OWS ";" OWS transfer-param )
    transfer-param      = token BWS "=" BWS ( token / quoted-string )

    quoted-string       = DQUOTE *( qdtext / quoted-pair ) DQUOTE
    qdtext              = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
    obs-text            = %x80-FF
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-4
        https://www.rfc-editor.org/errata/eid4839
        https://www.rfc-editor.org/errata/eid4891
*/
struct transfer_param_list_bnf
{
    std::pair<
        string_view,
        string_view> value;

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

using transfer_param_list =
    range<transfer_param_list_bnf>;

} // http_proto
} // boost

#endif
