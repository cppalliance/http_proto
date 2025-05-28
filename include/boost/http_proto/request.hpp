//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_HPP
#define BOOST_HTTP_PROTO_REQUEST_HPP

#include <boost/http_proto/request_base.hpp>

namespace boost {
namespace http_proto {

/** Container for HTTP requests
*/
class request
    : public request_base
{
public:
    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    request(
        core::string_view s);

    /** Constructor

        Construct a request container which allocates
        `storage_size` bytes for storing the header.
        Attempting to grow the container beyond
        this amount will result in an exception.
        The storage is also used internally to store
        instances of an implementation-defined type.
        The requested number of bytes will be aligned
        accordingly (currently the alignment requirement is 4).

        <br/>

        This constructor is useful when an upper-bound size
        of the request is known ahead of time and we want
        to prevent reallocations.

        <br/>

        Passing an initial storage size of `0` does not
        throw and the maximum capacity is set to an
        implementation-defined limit observable via
        @ref max_capacity_in_bytes().

        @param storage_size The initial and final size of
        the storage.

        @code
        boost::http_proto::request
        make_request(std::string_view host)
        {
            std::size_t size = 4096;
            // req.buffer() is now stable
            boost::http_proto::request req(size);
            BOOST_ASSERT(
                req.max_capacity_in_bytes(), 4096);

            // uses spare capacity so that reallocations
            // are avoided
            req.append(
                boost::http_proto::field::host, host);
            req.append(
                boost::http_proto::field::connection, "close");
            return req;
        }
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    request(
        std::size_t storage_size);

    /** Constructor

        Construct a request container which allocates
        `storage_size` bytes for storing the header, with an
        upper limit of `max_storage_size`. Attempting to
        grow the container beyond its maximum will result in
        an exception. The storage is also used internally to
        store instances of an implementation-defined type.
        Both values will be aligned accordingly (currently
        the alignment requirement is 4).

        <br/>

        This constructor is useful when there's a best-fit
        guess for an initial header size but we still wish
        to permit reallocating and growing the container to
        some upper limit.

        <br/>

        Passing an initial size of `0` does not throw.

        @param storage_size The initial size of the storage.

        @param max_storage_size The maximum size of the
        allocated storage. Any operation that attempts to
        grow the container beyond this value throws
        `std::length_error`.

        @throws std::length_error Thrown if `size > max_size`

        @code
        boost::http_proto::reqest
        make_reqest(std::string_view host)
        {
            std::size_t size = 4096;
            boost::http_proto::reqest req(size, 2 * size);
            BOOST_ASSERT(
                req.max_capacity_in_bytes(), 2 * 4096);

            // uses spare capacity so that reallocations
            // are avoided
            req.append(
                boost::http_proto::field::host, host);
            req.append(
                boost::http_proto::field::connection, "close");
            return req;
        }
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    request(
        std::size_t storage_size,
        std::size_t max_storage_size);

    /** Constructor

        The moved-from object will be
        left in the default-constructed
        state.
    */
    BOOST_HTTP_PROTO_DECL
    request(request&& other) noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request(request const& other);

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request(
        request_view const& other);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    request&
    operator=(request&&) noexcept;

    /** Assignment
    */
    request&
    operator=(
        request const& other)
    {
        copy_impl(*other.ph_);
        return *this;
    }

    /** Assignment
    */
    request&
    operator=(
        request_view const& other)
    {
        copy_impl(*other.ph_);
        return *this;
    }

    //--------------------------------------------

    /** Swap this with another instance
    */
    void
    swap(request& other) noexcept
    {
        h_.swap(other.h_);
    }

    /** Swap two instances
    */
    // hidden friend
    friend
    void
    swap(
        request& t0,
        request& t1) noexcept
    {
        t0.swap(t1);
    }
};

} // http_proto
} // boost

#endif
