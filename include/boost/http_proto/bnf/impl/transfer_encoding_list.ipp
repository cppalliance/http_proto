//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_TRANSFER_ENCODING_LIST_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_TRANSFER_ENCODING_LIST_IPP

#include <boost/http_proto/bnf/transfer_encoding_list.hpp>
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

/*
    legacy list rules:
    1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
*/

// *( "," OWS ) element *( OWS "," )
char const*
transfer_encoding_list_bnf::
begin(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    tchar_set ts;
    // *( "," OWS )
    auto const first =
        detail::skip_opt_comma_ows(
            start, end);
    // token
    auto it = ts.skip(first, end);
    if(it == first)
    {
        // missing token
        ec = error::bad_list;
        return start;
    }
    value.name = { first, static_cast<
        std::size_t>(it - first) };
    // transfer-param-list
    auto const s = valid_prefix<
        transfer_param_list_bnf>({ it,
            static_cast<std::size_t>(
                end - it) });
    value.params = transfer_param_list(s);
    // *( OWS "," )
    return detail::skip_opt_ows_comma(
        comma_, &*s.end(), end);
}

// [ OWS element *( OWS "," ) ]
char const*
transfer_encoding_list_bnf::
increment(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    ws_set ws;
    tchar_set ts;

    // [ ... ]
    if(start == end)
    {
        ec = error::end;
        return end;
    }
    if(! comma_)
    {
        // missing comma
        ec = error::bad_list;
        return start;
    }
    // OWS
    auto const first =
        ws.skip(start, end);
    // token
    auto it = ts.skip(first, end);
    if(it == first)
    {
        // missing token
        ec = error::bad_list;
        return start;
    }
    value.name = { first, static_cast<
        std::size_t>(it - first) };
    // transfer-param-list
    auto const s = valid_prefix<
        transfer_param_list_bnf>({ it,
            static_cast<std::size_t>(
                end - it) });
    if(ec)
        return start;
    value.params = transfer_param_list(s);
//  *( OWS "," )
    return detail::skip_opt_ows_comma(
        comma_, &*s.end(), end);
}

} // http_proto
} // boost

#endif
