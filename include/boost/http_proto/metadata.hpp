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
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/error.hpp> // VFALCO TEMPORARY
#include <cstdint>
#include <cstdlib>

namespace boost {
namespace http_proto {

//------------------------------------------------

/** Metadata about the payload in a message
*/
struct payload
{
    // VFALCO 3 space indent or
    // else Doxygen malfunctions
    enum what
    {
        /**
          * This message has no payload
        */
        none

        /**
          * This message has a known payload size
        */
        ,sized

        /**
          * The payload for this message continues until EOF
        */
        ,to_eof

        /**
          * This message contains a chunked payload
        */
        ,chunked

        /**
          * The payload is unknown due to errors
        */
        ,error
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

//------------------------------------------------

/** Metadata for the Transfer-Encoding field
*/
struct transfer_encoding
{
    /** The total number of fields
    */
    std::size_t count = 0;

    /** The total number of codings
    */
    std::size_t codings = 0;

    /** True if valid and chunked is specified
    */
    bool is_chunked = false;

    /** True if the fields are invalid
    */
    bool error = false;

#ifndef BOOST_HTTP_PROTO_DOCS
    // workaround for C++ aggregate init
    constexpr
    transfer_encoding() = default;

    constexpr
    transfer_encoding(
        std::size_t count_,
        std::size_t codings_,
        bool is_chunked_,
        bool error_) noexcept
        : count(count_)
        , codings(codings_)
        , is_chunked(is_chunked_)
        , error(error_)
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

//------------------------------------------------

/** Metadata about a request or response
*/
struct metadata
{
    /** Metadata for the Content-Length field
    */
    struct content_length_t
    {
        /** Error status of Content-Length
        */
        //error_code ec;
        error ec = error::success;

        /** The total number of fields
        */
        std::size_t count = 0;

        /** True if value is set
        */
        bool has_value = false;

        /** The value as an integer
        */
        std::uint64_t value = 0;

    #ifndef BOOST_HTTP_PROTO_DOCS
        // workaround for C++ aggregate init
        constexpr
        content_length_t() = default;

        constexpr
        content_length_t(
            error ec_,
            std::size_t count_,
            bool has_value_,
            std::uint64_t value_) noexcept
            : ec(ec_)
            , count(count_)
            , has_value(has_value_)
            , value(value_)
        {
        }
    #endif
    };

    content_length_t content_length;
    http_proto::connection connection;
    http_proto::transfer_encoding transfer_encoding;
    http_proto::upgrade upgrade;
};

} // http_proto
} // boost

#endif
