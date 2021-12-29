//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_CHUNK_PART_HPP
#define BOOST_HTTP_PROTO_RULE_CHUNK_PART_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/rfc/chunk_ext_rule.hpp>
#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/url/grammar/range.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** Base class for chunk part BNFs.
*/
class chunk_part_base
{
public:
    struct value_type
    {
        std::uint64_t size; // 0=final
        std::size_t prefix_size; // including CRLF
        grammar::range<chunk_ext_rule> ext;
        grammar::range<field_rule> trailer;
        string_view data;
    };

    value_type const&
    value() const noexcept
    {
        return v_;
    }

protected:
    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec);

    value_type v_;
};

/** BNF for parsing parts of chunked encodings

    This is actually just the prefix of a chunk,
    last chunk, or trailer part, which may include
    some or none of the body even on a successful parse.

    @par BNF
    @code
    chunk-part      = ( chunk / ( last-chunk trailer-part CRLF ) )
    chunk           = chunk-size [ chunk-ext ] CRLF chunk-data
    chunk-size      = 1*HEXDIG
    chunk-data      = 1*OCTET
    last-chunk      = 1*("0") [ chunk-ext ] CRLF
    trailer-part    = *( header-field CRLF )
    @endcode

    @see
        @ref chunk_ext
        @ref header_fields
        @ref hex_number
        https://datatracker.ietf.org/doc/html/rfc7230#section-4.1.1
*/
class chunk_part
    : public chunk_part_base
{
public:
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec)
    {
        return chunk_part_base::parse(
            start, end, ec);
    }
};

/** BNF for parsing parts of chunked encodings

    @par BNF
    @code
    chunk-part      = CRLF ( chunk / ( last-chunk trailer-part CRLF ) )
    chunk           = chunk-size [ chunk-ext ] CRLF chunk-data CRLF
    chunk-size      = 1*HEXDIG
    chunk-data      = 1*OCTET
    last-chunk      = 1*("0") [ chunk-ext ] CRLF
    trailer-part    = *( header-field CRLF )
    @endcode

    @see
        @ref chunk_ext
        @ref header_fields
        @ref hex_number
        https://datatracker.ietf.org/doc/html/rfc7230#section-4.1.1
*/
class chunk_part_next
    : public chunk_part_base
{
public:
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec)
    {
        auto it = consume_crlf(
            start, end, ec);
        if(ec.failed())
            return it;
        it = chunk_part_base::parse(
            it, end, ec);
        if(! ec)
            v_.prefix_size += 2; // for CRLF
        return it;
    }
};

} // bnf
} // http_proto
} // boost

#endif
