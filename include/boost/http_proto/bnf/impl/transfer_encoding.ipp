//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_IMPL_TRANSFER_ENCODING_LIST_IPP
#define BOOST_HTTP_PROTO_RULE_IMPL_TRANSFER_ENCODING_LIST_IPP

#include <boost/http_proto/bnf/transfer_encoding.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/ctype.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
transfer_coding::
parse(
    char const* start,
    char const* const end,
    error_code& ec)
{
    // token
    token t;
    auto it = t.parse(
        start, end, ec);
    if(ec.failed())
        return it;
    v_.name = t.value();
    // transfer-param-list
    start = it;
    it = consume<
        transfer_param_list>(
            it, end, ec);
    if(ec == grammar::error::incomplete)
        return it;
    if(ec.failed())
    {
        // leave it out
        ec = {};
        return start;
    }
    v_.params = range<
        transfer_param_list>(
            string_view(start,
                it - start));
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
