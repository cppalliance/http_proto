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
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/bnf/sequence.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for transfer-param-elem

    Parameters are in the form of a name=value pair.

    @par BNF
    @code
    transfer-param-elem = OWS ";" OWS token OWS "=" OWS ( token / quoted-string )
    @endcode

    @see
        @ref quoted_string
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-4
        https://www.rfc-editor.org/errata/eid4839
        https://www.rfc-editor.org/errata/eid4891
*/
class transfer_param_elem
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

    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec);

private:
    value_type v_;
};

/** BNF for transfer-param-list

    Parameters are in the form of a name=value pair.

    @par BNF
    @code
    transfer-param-list = *transfer-param-elem
    @endcode

    @see
        @ref transfer_param
        https://datatracker.ietf.org/doc/html/rfc5234
*/
using transfer_param_list =
    zero_or_more<transfer_param_elem>;

} // bnf
} // http_proto
} // boost

#endif
