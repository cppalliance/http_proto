//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_FILTER_HPP
#define BOOST_HTTP_PROTO_DETAIL_FILTER_HPP

#include <boost/buffers/const_buffer_pair.hpp>
#include <boost/buffers/mutable_buffer_subspan.hpp>
#include <boost/system/error_code.hpp>

namespace boost {
namespace http_proto {
namespace detail {

/** Base class for all filters
*/
class filter
{
public:
    /** The results of processing the filter.
    */
    struct results
    {
        /** The error, if any occurred.
        */
        system::error_code ec;

        /** The number of bytes produced in the output.

            This may be less than the total number
            of bytes available for writing in the
            destination buffers.
        */
        std::size_t out_bytes = 0;

        /** The number of bytes consumed from the input.

            This may be less than the total number
            of bytes available for reading in the
            source buffers.
        */
        std::size_t in_bytes = 0;

        /** True if the output buffer is too
            small to make progress.

            This can only happen in deflate operation.
        */
        bool out_short = false;

        /** True if there will be no more output.
        */
        bool finished = false;
    };

    results
    process(
        buffers::mutable_buffer_subspan out,
        buffers::const_buffer_pair in,
        bool more,
        bool partial_flush = false);

protected:
    enum class flush
    {
        none,
        partial,
        finish
    };

    virtual
    std::size_t
    min_out_buffer() const noexcept
    {
        return 0;
    }

    virtual
    results
    do_process(
        buffers::mutable_buffer,
        buffers::const_buffer,
        flush) noexcept = 0;
};

} // detail
} // http_proto
} // boost

#endif
