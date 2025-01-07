//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SINK_HPP
#define BOOST_HTTP_PROTO_SINK_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffered_base.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/system/error_code.hpp>
#include <cstddef>
#include <type_traits>

namespace boost {
namespace http_proto {

/** An algorithm for consuming buffers of data.

    This interface abstracts the consumption of
    a finite stream of data, passed by reading
    from caller-provided buffers until there
    is no more input data.

    @par Thread Safety
    Non-const member functions may not be
    called concurrently on the same instance.
*/
struct BOOST_HTTP_PROTO_DECL
    sink
{
    /** The results of consuming data.
    */
    struct results
    {
        /** The error, if any occurred.
        */
        system::error_code ec;

        /** The number of bytes consumed in the input.
        */
        std::size_t bytes = 0;

        /** Accumulate results.
        */
        results&
        operator+=(
            results const& rv) noexcept;
    };

    /** Consume data.

        This function attempts to write to the
        sink, by transferring data from the given
        constant buffer sequence.
        The return value indicates the number of
        bytes consumed from the buffers and the
        error if any occurred.

        @par Preconditions
        @li @ref init was called, and
        @li This is the first call to @ref write,
            or the last value of `more` was `true`.

        @par Postconditions
        @code
        rv.ec.failed() == true || rv.bytes == buffer_size(bs)
        @endcode

        @return The result of the operation.

        @param bs The buffers to use.
            Each buffer in the sequence will be
            consumed completely before the next
            buffer is accessed.

        @param more `true` if there will be one
            or more subsequent calls to @ref write.
    */
    template<class ConstBufferSequence>
    results
    write(
        ConstBufferSequence const& bs,
        bool more)
    {
        static_assert(
            buffers::is_const_buffer_sequence<
                ConstBufferSequence>::value,
            "Type requirements not met");

        return write_impl(bs, more);
    }

#ifdef BOOST_HTTP_PROTO_DOCS
protected:
#else
private:
#endif
    /** Derived class override.

        This pure virtual function is called by
        the implementation and must be overriden.
        The callee should attempt to consume data
        from the given constant buffer.
        The return value must be set to indicate
        the number of bytes consumed from the
        buffers, and the error if any occurred.

        @par Preconditions
        @li @ref init was called, and
        @li This is the first call to @ref on_write,
            or the last value of `more` was `true`.

        @return The result of the operation.

        @param b The buffer to use.
            If `more` is true then the results
            must indicate that the buffer was
            consumed completely, or that an error
            occurred.

        @param more `true` if there will be one
            or more subsequent calls to @ref write.
    */
    virtual
    results
    on_write(
        buffers::const_buffer b,
        bool more) = 0;

    /** Derived class override.

        This pure virtual function is called by
        the implementation and must be overriden.
        The callee should attempt to consume data
        from the given constant buffer sequence.
        The return value must be set to indicate
        the number of bytes consumed from the
        buffers, and the error if any occurred.

        @par Preconditions
        @li @ref init was called, and
        @li This is the first call to @ref on_write,
            or the last value of `more` was `true`.

        @return The result of the operation.

        @param bs The buffer sequence to use.
            Each buffer in the sequence must
            be completely consumed before data
            is consumed from the next buffer.
            If `more` is true then the results
            must indicate that the buffer was
            consumed completely, or that an error
            occurred.

        @param more `true` if there will be one
            or more subsequent calls to @ref write.
    */
    virtual
    results
    on_write(
        buffers::const_buffer_span bs,
        bool more);

private:
    results
    write_impl(
        buffers::const_buffer const& b,
        bool more)
    {
        return on_write(b, more);
    }

    results
    write_impl(
        buffers::mutable_buffer const& b,
        bool more)
    {
        return on_write(b, more);
    }

    results
    write_impl(
        buffers::const_buffer_span const& bs,
        bool more)
    {
        return on_write(bs, more);
    }

    template<class T>
    results
    write_impl(T const&, bool);
};

//------------------------------------------------

/** Metafunction which determines if T is a sink

    @see
        @ref sink.
*/
#ifdef BOOST_HTTP_PROTO_DOCS
template<class T>
using is_sink = __see_below__;
#else
template<class T>
using is_sink =
    std::is_convertible<
        typename std::decay<T>::type*,
        sink*>;
#endif

} // http_proto
} // boost

#include <boost/http_proto/impl/sink.hpp>

#endif
