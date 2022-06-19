//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILTER_HPP
#define BOOST_HTTP_PROTO_FILTER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <cstdlib>

#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** A source provides buffers of data.
*/
struct BOOST_SYMBOL_VISIBLE
    source
{
    virtual ~source() = 0;

    virtual
    bool
    more() const noexcept = 0;

    /** Return prepared buffers for reading

        @param ec Set to the error, if any occurred
    */
    virtual
    const_buffers
    prepare(error_code& ec) = 0;

    /** Consume output from the source

        @param n The number of bytes
    */
    virtual
    void
    consume(std::size_t n) noexcept = 0;
};

//------------------------------------------------

/** A sink consumes buffers of data.
*/
struct BOOST_SYMBOL_VISIBLE
    sink
{
    virtual ~sink() = 0;

    /** Consume a set of buffers

        @param ec Set to the error, if any occurred
    */
    virtual
    std::size_t
    write(
        const_buffers cb,
        error_code& ec ) = 0;

    /** Indicate that no more buffers are coming

        @param ec Set to the error, if any occurred
    */
    virtual
    void
    write_eof(
        error_code& ec ) = 0;
};

//------------------------------------------------

/**
    A filter processes as much data as possible,
    and stops when the input is consumed or the
    internal output buffer becomes full. It may
    introduce some output latency (reading input
    without producing any output) except when
    forced to flush.
*/
struct BOOST_SYMBOL_VISIBLE
    filter : source, sink
{
};

} // http_proto
} // boost

#endif
