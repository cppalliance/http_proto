//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_IMPL_CHUNK_PART_IPP
#define BOOST_HTTP_PROTO_RULE_IMPL_CHUNK_PART_IPP

#include <boost/http_proto/bnf/chunk_part.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
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
    if(ec.failed())
        return it;
    v_.size = hn.value();
    // [ chunk-ext ]
    if(! grammar::parse(
        it, end, ec, v_.ext))
        return it;

    // CRLF
    it = consume_crlf(it, end, ec);
    if(ec.failed())
        return it;
    // chunk / last-chunk...
    if(v_.size > 0)
    {
        // chunk
        std::size_t n = end - it;
        v_.trailer = {};
        v_.prefix_size =
            it - start;
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
    if(! grammar::parse(
        it, end, ec, v_.trailer))
        return it;
    v_.data = {};
    return it;
}

} // bnf
} // http_proto
} // boost

#endif
