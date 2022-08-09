//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_METADATA_HPP
#define BOOST_HTTP_PROTO_METADATA_HPP

#include <boost/http_proto/detail/config.hpp>
#include <cstdint>
#include <cstdlib>

namespace boost {
namespace http_proto {

//------------------------------------------------

/** Metadata about the payload in a message
*/
struct payload
{
    enum what
    {
        none,
        sized,
        to_eof,
        chunked
    };

    what kind = what::none;
    std::uint64_t size = 0;
};

//------------------------------------------------

/** Metadata for the Connection field
*/
struct connection
{
    /** The total number of fields
    */
    std::size_t count = 0;

    /** true if the close token was seen
    */
    std::size_t n_close = 0;

    /** Number of times keep-alive was seen
    */
    std::size_t n_keepalive = 0;

    /** Number of times upgrade token was seen
    */
    std::size_t n_upgrade = 0;

    /** True if any parse error occurred
    */
    bool error = false;

#ifndef BOOST_HTTP_PROTO_DOCS
    // workaround for C++ aggregate init
    constexpr
    connection() = default;

    constexpr
    connection(
        std::size_t count_,
        std::size_t n_close_,
        std::size_t n_keepalive_,
        std::size_t n_upgrade_,
        bool error_) noexcept
        : count(count_)
        , n_close(n_close_)
        , n_keepalive(n_keepalive_)
        , n_upgrade(n_upgrade_)
        , error(error_)
    {
    }
#endif
};

/** Metadata for the Content-Length field
*/
struct content_length
{
    /** The total number of fields
    */
    std::size_t count = 0;

    /** The value as an integer, if valid
    */
    std::uint64_t value = 0;

    /** True if fields are valid and no overflow
    */
    bool has_value = false;

#ifndef BOOST_HTTP_PROTO_DOCS
    // workaround for C++ aggregate init
    constexpr
    content_length() = default;

    constexpr
    content_length(
        std::size_t count_,
        std::uint64_t value_,
        bool has_value_) noexcept
        : count(count_)
        , value(value_)
        , has_value(has_value_)
    {
    }
#endif
};

//------------------------------------------------

/** Metadata for the Transfer-Encoding field
*/
struct transfer_encoding
{
    /** The total number of fields
    */
    std::size_t count = 0;

    /** The number of times "chunked" appears
    */
    std::size_t chunked_count = 0;

    /** True if valid and chunked is specified
    */
    bool is_chunked = false;

#ifndef BOOST_HTTP_PROTO_DOCS
    // workaround for C++ aggregate init
    constexpr
    transfer_encoding() = default;

    constexpr
    transfer_encoding(
        std::size_t count_,
        std::size_t chunked_count_,
        bool is_chunked_) noexcept
        : count(count_)
        , chunked_count(chunked_count_)
        , is_chunked(is_chunked_)
    {
    }
#endif
};

//------------------------------------------------

/** Metadata for Upgrade field
*/
struct upgrade
{
    /** The total number of fields
    */
    std::size_t count = 0;

    /** True if websocket appears at least once
    */
    bool websocket = false;
};

} // http_proto
} // boost

#endif
