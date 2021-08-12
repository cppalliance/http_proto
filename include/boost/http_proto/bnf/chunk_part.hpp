//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_CHUNK_PART_HPP
#define BOOST_HTTP_PROTO_BNF_CHUNK_PART_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/chunk_ext.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for parsing parts of chunked encodings

    This is actually just the prefix of a chunk,
    last chunk, or trailer part, which may include
    some or none of the body even on a successful parse.

    @par BNF
    @code
    chunk-part      = ( chunk / ( last-chunk trailer-part CRLF ) )
    chunk           = chunk-size [ chunk-ext ] CRLF chunk-data CRLF
    chunk-size      = 1*HEXDIG
    chunk-data      = 1*OCTET
    last-chunk      = 1*("0") [ chunk-ext ] CRLF
    trailer-part    = header-fields
    @endcode

    @see
        @ref chunk_ext
        @ref header_fields
        @ref hex_number
        https://datatracker.ietf.org/doc/html/rfc7230#section-4.1.1
*/
class chunk_part
{
public:
    struct value_type
    {
        std::uint64_t size;
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

} // bnf
} // http_proto
} // boost

#endif
