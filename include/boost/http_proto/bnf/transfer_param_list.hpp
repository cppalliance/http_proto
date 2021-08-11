//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_TRANSFER_PARAM_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_TRANSFER_PARAM_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/string_view.hpp>
#include <utility>

namespace boost {
namespace http_proto {
namespace bnf {

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
class transfer_param_list
{
public:
    struct value_type
    {
        string_view name;
        string_view value;
    };

    value_type const&
    value() const noexcept
    {
        return v_;
    }

    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec)
    {
        return increment(
            start, end, ec);
    }

    BOOST_HTTP_PROTO_DECL
    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec);

private:
    value_type v_;
};

} // bnf
} // http_proto
} // boost

#endif
