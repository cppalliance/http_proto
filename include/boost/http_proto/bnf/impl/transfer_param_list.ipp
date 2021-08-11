//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_TRANSFER_PARAM_LIST_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_TRANSFER_PARAM_LIST_IPP

#include <boost/http_proto/bnf/transfer_param_list.hpp>
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/*
    transfer-param-list = *( OWS ";" OWS transfer-param )
    transfer-param      = token BWS "=" BWS ( token / quoted-string )
*/

char const*
transfer_param_list_bnf::
begin(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    return increment(
        start, end, ec);
}

char const*
transfer_param_list_bnf::
increment(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    ws_set ws;
    tchar_set ts;

    // *( ... )
    if(start == end)
    {
        ec = error::end;
        return end;
    }
    // OWS
    auto it =
        ws.skip(start, end);
    // ";"
    if(it == end)
    {
        ec = error::bad_list;
        return start;
    }
    if(*it != ';')
    {
        ec = error::bad_list;
        return start;
    }
    ++it;
    // OWS
    it = ws.skip(it, end);
    // token
    auto t0 = it;
    it = ts.skip(t0, end);
    if(it == t0)
    {
        ec = error::bad_list;
        return start;
    }
    value.name = {
        t0, static_cast<
            std::size_t>(it - t0) };
    // OWS
    it = ws.skip(it, end);
    // "="
    if(it == end)
    {
        ec = error::bad_list;
        return start;
    }
    if(*it != '=')
    {
        ec = error::bad_list;
        return start;
    }
    ++it;
    // OWS
    it = ws.skip(it, end);
    // token
    t0 = it;
    it = ts.skip(t0, end);
    if(it == t0)
    {
        // value must be present
        // https://www.rfc-editor.org/errata/eid4839
        ec = error::bad_list;
        return start;
    }
    value.value = {
        t0, static_cast<
            std::size_t>(it - t0) };
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
