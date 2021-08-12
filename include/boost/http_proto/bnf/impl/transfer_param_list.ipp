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
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/quoted_string.hpp>
#include <boost/http_proto/bnf/token.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
transfer_param_list::
increment(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    // *( ... )
    if(start == end)
    {
        ec = error::end;
        return end;
    }
    // OWS
    ws_set ws;
    auto it =
        ws.skip(start, end);
    // ";"
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != ';')
    {
        // expected ';'
        ec = error::syntax;
        return start;
    }
    ++it;
    // OWS
    it = ws.skip(it, end);
    // token
    token t;
    it = t.parse(it, end, ec);
    if(ec)
        return it;
    v_.name = t.value();
    // OWS
    it = ws.skip(it, end);
    // "="
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != '=')
    {
        // expected "="
        ec = error::syntax;
        return start;
    }
    ++it;
    // OWS
    it = ws.skip(it, end);
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    // ( token / quoted-string )
    it = t.parse(it, end, ec);
    if(! ec)
    {
        // token
        v_.value = t.value();
        return it;
    }
    ec = {};
    quoted_string q;
    it = q.parse(it, end, ec);
    if(ec == error::need_more)
        return it;
    if(! ec)
    {
        // quoted-string
        v_.value = q.value();
        return it;
    }
    // value must be present
    // https://www.rfc-editor.org/errata/eid4839
    ec = error::syntax;
    return start;
}

} // bnf
} // http_proto
} // boost

#endif
