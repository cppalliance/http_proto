//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_PART_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_PART_IPP

#include <boost/http_proto/bnf/chunk_part.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/number.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
chunk_part_base::
parse(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    // chunk-size
    hex_number hn;
    auto it = hn.parse(
        start, end, ec);
    if(ec)
        return it;
    v_.size = hn.value();
    // [ chunk-ext ]
    auto p = it;
    it = consume<chunk_ext>(
        it, end, ec);
    if(ec)
        return it;
    v_.ext = range<chunk_ext>(
        string_view(p, it - p));
    // CRLF
    it = consume_crlf(it, end, ec);
    if(ec)
        return it;
    // chunk / last-chunk...
    if(v_.size > 0)
    {
        // chunk
        std::size_t n = end - it;
        if(n > v_.size)
        {
            // complete body
            v_.data = string_view(
                it, v_.size);
            it += v_.size;
            // Leave the final CRLF
            // for the next parsed
            // part to consume
            return it;
        }
        // partial body
        v_.data = string_view(it, n);
        it += n;
        return it;
    }
    // last-chunk trailer-part CRLF
    // (includes the last CRLF)
    p = it;
    it = consume<header_fields>(
        it, end, ec);
    if(ec)
        return it;
    v_.trailer =
        range<header_fields>(
            string_view(p, it - p));
    v_.data = {};
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
