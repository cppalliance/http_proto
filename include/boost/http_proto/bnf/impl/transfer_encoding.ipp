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

#include <boost/http_proto/bnf/transfer_encoding.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/ctype.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
transfer_coding::
parse(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    tchar_set ts;
    // token
    auto it = ts.skip(start, end);
    if(it == start)
    {
        // missing token
        ec = error::bad_list;
        return start;
    }
    v_.name = string_view(
        start, it - start);
    // transfer-param-list
    auto const s = valid_prefix<
        transfer_param_list>({ it,
            static_cast<std::size_t>(
                end - it) });
    v_.params = range<transfer_param_list>(s);
    it = s.data() + s.size();
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
