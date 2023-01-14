//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SOURCE_HPP
#define BOOST_HTTP_PROTO_SOURCE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** A source of body data
*/
struct BOOST_SYMBOL_VISIBLE
    source
{
    /** A function to reserve a buffer

        This function object may be called
        up to one time to reserve intermediate
        storage.
    */
    struct BOOST_SYMBOL_VISIBLE
        reserve_fn
    {
        /** Return a buffer of at least n bytes.

            This function reserves space for
            a buffer of at least `n` bytes in
            size. If there is insufficient space
            available, an exception is thrown.
            Undefined behavior results if this
            function is invoked more than once.

            @par Exceptions
            Calls to allocate may throw.

            @param n The size of the buffer.
        */
        BOOST_HTTP_PROTO_DECL
        virtual
        void*
        operator()(
            std::size_t n) const = 0;

    protected:
        virtual ~reserve_fn() = 0;
    };

    /** The results of reading from the source.
    */
    struct results
    {
        error_code ec;
        std::size_t bytes = 0;
        bool more = false;
    };

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    ~source() = 0;

    /** Called to allow the source to reserve memory.

        This function is invoked once before
        serialization begins to give the source
        an opportunity to reserve temporary
        storage. The default implementation
        does nothing.
        <br>
        The `reserve` function object only
        remains valid until `maybe_reserve`
        returns.

        @param limit The maximum number of bytes
        which may be reserved.

        @param reserve A function object to
        invoke up to one time in order to
        obtain a buffer.
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    void
    maybe_reserve(
        std::size_t limit,
        reserve_fn const& reserve);

    /** Called when more data is required

        This function is invoked when more data
        is required. The subclass should place
        zero or more bytes into the buffers
        referenced by `dest`, and return a
        `results` value with the members set
        appropriately.
        <br>
        Partial success is possible.
    */
    virtual
    results
    read(
        mutable_buffers_pair dest) = 0;
};

/** Metafunction which determines if T is a source

    @see
        @ref source.
*/
#ifdef BOOST_HTTP_PROTO_DOCS
template<class T>
using is_source = __see_below__;
#else
template<class T>
using is_source =
    std::is_convertible<
        typename std::decay<T>::type*,
        source*>;
#endif

} // http_proto
} // boost

#endif
