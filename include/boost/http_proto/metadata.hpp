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

/** Identifies the payload type of a message
*/
enum class payload
{
// VFALCO 3 space indent or
// else Doxygen malfunctions

    /**
      * This message has no payload
    */
    none

    /**
      * The payload is unknown due to errors
    */
    ,error

    /**
      * This message has a known payload size

        The function @ref message_view_base::payload_size
        may be used to obtain the exact number of
        octets in the actual payload.
    */
    ,size

    /**
      * The payload for this message continues until EOF
    */
    ,to_eof

    /**
      * This message contains a chunked payload
    */
    ,chunked
};

//------------------------------------------------

/** Metadata about a request or response
*/
struct metadata
{
    /** Metadata for the Connection field
    */
    struct connection_t
    {
        /** Error status of Connection
        */
        error_code ec;

        /** The total number of fields
        */
        std::size_t count = 0;

        /** true if a close token is present
        */
        bool close = false;

        /** true if a keep-alive token is present
        */
        bool keep_alive = false;

        /** true if an upgrade token is present
        */
        bool upgrade = false;

    #ifndef HTTP_PROTO_DOCS
        constexpr
        connection_t() = default;
    #endif

    #if defined(BOOST_NO_CXX14_AGGREGATE_NSDMI) || defined(BOOST_MSVC)
        constexpr
        connection_t(
            error_code ec_,
            std::size_t count_,
            bool close_,
            bool keep_alive_,
            bool upgrade_) noexcept
            : ec(ec_)
            , count(count_)
            , close(close_)
            , keep_alive(keep_alive_)
            , upgrade(upgrade_)
        {
        }
    #endif
    };

    /** Metadata for the Content-Length field
    */
    struct content_length_t
    {
        /** Error status of Content-Length
        */
        error_code ec;

        /** The total number of fields
        */
        std::size_t count = 0;

        /** The value as an integer

            This is only valid when ec does
            not hold a failure, and when
            count is greater than zero.
        */
        std::uint64_t value = 0;

    #ifndef HTTP_PROTO_DOCS
        constexpr
        content_length_t() = default;
    #endif

    #if defined(BOOST_NO_CXX14_AGGREGATE_NSDMI) || defined(BOOST_MSVC)
        constexpr
        content_length_t(
            error_code ec_,
            std::size_t count_,
            std::uint64_t value_) noexcept
            : ec(ec_)
            , count(count_)
            , value(value_)
        {
        }
    #endif
    };

    /** Metadata for the Transfer-Encoding field
    */
    struct transfer_encoding_t
    {
        /** Error status of Content-Length
        */
        error_code ec;

        /** The total number of fields
        */
        std::size_t count = 0;

        /** The total number of codings
        */
        std::size_t codings = 0;

        /** True if valid and chunked is specified last
        */
        bool is_chunked = false;

    #ifndef HTTP_PROTO_DOCS
        constexpr
        transfer_encoding_t() = default;
    #endif

    #if defined(BOOST_NO_CXX14_AGGREGATE_NSDMI) || defined(BOOST_MSVC)
        constexpr
        transfer_encoding_t(
            error_code ec_,
            std::size_t count_,
            std::size_t codings_,
            bool is_chunked_) noexcept
            : ec(ec_)
            , count(count_)
            , codings(codings_)
            , is_chunked(is_chunked_)
        {
        }
    #endif
    };

    /** Metadata for Upgrade field
    */
    struct upgrade_t
    {
        /** Error status of Upgrade
        */
        error_code ec;

        /** The total number of fields
        */
        std::size_t count = 0;

        /** True if websocket appears at least once
        */
        bool websocket = false;

    #ifndef HTTP_PROTO_DOCS
        constexpr
        upgrade_t() = default;
    #endif

    #if defined(BOOST_NO_CXX14_AGGREGATE_NSDMI) || defined(BOOST_MSVC)
        constexpr
        upgrade_t(
            error_code ec_,
            std::size_t count_,
            bool websocket_) noexcept
            : ec(ec_)
            , count(count_)
            , websocket(websocket_)
        {
        }
    #endif
    };

    constexpr metadata() = default;

    //--------------------------------------------

    /** True if payload is manually specified
    */
    bool manual_payload = false;

    /** The type of payload
    */
    http_proto::payload payload =
        http_proto::payload::none;

    /** The size of the payload if known

        This is only valid when @ref payload
        equals @ref http_proto::payload::size.
    */
    std::uint64_t payload_size = 0;

    upgrade_t upgrade;
    connection_t connection;
    content_length_t content_length;
    transfer_encoding_t transfer_encoding;
};

} // http_proto
} // boost

#endif
