//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

#ifndef BOOST_HTTP_PROTO_FILTER_HPP
#define BOOST_HTTP_PROTO_FILTER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffered_base.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/mutable_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/system/error_code.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

/** A filter for buffers
*/
class BOOST_HTTP_PROTO_DECL
    filter
    : public buffered_base
{
    friend class serializer;

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

//------------------------------------------------

/** Metafunction which determines if T is a filter

    @see
        @ref source.
*/
#ifdef BOOST_BUFFERS_DOCS
template<class T>
using is_filter = __see_below__;
#else
template<class T>
using is_filter =
    std::is_convertible<
        typename std::decay<T>::type*,
        filter*>;
#endif

} // http_proto
} // boost

#include <boost/http_proto/impl/filter.hpp>

#endif
