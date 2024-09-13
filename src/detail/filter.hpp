//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

#ifndef BOOST_HTTP_PROTO_DETAIL_FILTER_HPP
#define BOOST_HTTP_PROTO_DETAIL_FILTER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/mutable_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/system/error_code.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {
namespace detail {

/** A filter for buffers
*/
class BOOST_HTTP_PROTO_DECL
    filter
{
    template<
        class T,
        std::size_t N>
    class unrolled;

public:
    /** The results of processing the filter.
    */
    struct results
    {
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

        /** The error, if any occurred.
        */
        system::error_code ec;

        /** True if there will be no more output.
        */
        bool finished = false;
    };

    /** Called to process the filter.

        @par Preconditions
        @ref init was called once before any
            calls to `process`.
    */
    template<
        class MutableBufferSequence,
        class ConstBufferSequence>
    results
    process(
        MutableBufferSequence const& out,
        ConstBufferSequence const& in,
        bool more)
    {
        static_assert(
            buffers::is_mutable_buffer_sequence<
                MutableBufferSequence>::value,
            "Type requirements not met");

        static_assert(
            buffers::is_const_buffer_sequence<
                ConstBufferSequence>::value,
            "Type requirements not met");

        return process_impl(out, in, more);
    }

#ifdef BOOST_BUFFERS_DOCS
protected:
#else
private:
#endif
    /** Derived class override.

        @par Preconditions
        @ref init was called once before any
            calls to `process`
    */
    virtual
    results
    on_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) = 0;

    /** Called to process the filter.

        @par Preconditions
        @ref init was called once before any
            calls to `process`.
    */
    virtual
    results
    on_process(
        buffers::mutable_buffer_span out,
        buffers::const_buffer_span in,
        bool more);

private:
    results
    process_impl(
        buffers::mutable_buffer const& out,
        buffers::const_buffer const& in,
        bool more)
    {
        return on_process(out, in, more);
    }

    results
    process_impl(
        buffers::mutable_buffer_span const& out,
        buffers::const_buffer_span const& in,
        bool more)
    {
        return on_process(out, in, more);
    }

    template<
        class MutableBuffers,
        class ConstBuffers>
    results
    process_impl(
        MutableBuffers const& out,
        ConstBuffers const& in,
        bool more);
};

} // detail
} // http_proto
} // boost

#include "impl/filter.hpp"

#endif
