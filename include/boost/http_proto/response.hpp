//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_HPP
#define BOOST_HTTP_PROTO_RESPONSE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/status.hpp>

namespace boost {
namespace http_proto {

/** Container for HTTP responses
*/
class BOOST_SYMBOL_VISIBLE
    response
    : public message_base
{
public:
    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response(
        core::string_view s);

    /** Constructor

        Construct a response container which allocates
        `storage_size` bytes for storing the header.
        Attempting to grow the container beyond
        this amount will result in an exception.
        The storage is also used internally to store
        instances of an implementation-defined type.
        The requested number of bytes will be aligned
        accordingly (currently the alignment requirement is 4).

        <br/>

        This constructor is useful when an upper-bound size
        of the response is known ahead of time and we want
        to prevent reallocations.

        <br/>

        Passing an initial storage size of `0` does not
        throw and the maximum capacity is set to an
        implementation-defined limit observable via
        @ref max_capacity_in_bytes().

        @param storage_size The initial and final size of
        the storage.

        @code
        boost::http_proto::response
        make_response(std::string_view server)
        {
            std::size_t size = 4096;
            // res.buffer() is now stable
            boost::http_proto::response res(size);
            BOOST_ASSERT(
                res.max_capacity_in_bytes(), 4096);

            // uses spare capacity so that reallocations
            // are avoided
            res.append(
                boost::http_proto::field::server, server);
            res.append(
                boost::http_proto::field::connection, "close");
            return res;
        }
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response(
        std::size_t storage_size);

    /** Constructor

        Construct a response container which allocates
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
        boost::http_proto::response
        make_response(std::string_view host)
        {
            std::size_t size = 4096;
            boost::http_proto::response res(size, 2 * size);
            BOOST_ASSERT(
                res.max_capacity_in_bytes(), 2 * 4096);

            // uses spare capacity so that reallocations
            // are avoided
            res.append(
                boost::http_proto::field::host, host);
            res.append(
                boost::http_proto::field::connection, "close");
            return res;
        }
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    response(
        std::size_t storage_size,
        std::size_t max_storage_size);

    /** Constructor

        The moved-from object will be
        left in the default-constructed
        state.
    */
    BOOST_HTTP_PROTO_DECL
    response(response&& other) noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response(response const& other);

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response(
        response_view const& other);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    response&
    operator=(
        response&& other) noexcept;

    /** Assignment
    */
    response&
    operator=(
        response const& other)
    {
        copy_impl(*other.ph_);
        return *this;
    }

    /** Assignment
    */
    response&
    operator=(
        response_view const& other)
    {
        copy_impl(*other.ph_);
        return *this;
    }

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response(
        http_proto::status sc,
        http_proto::version v);

    /** Constructor
    *
    * The start-line of the response will contain the standard
    * text for the supplied status code and the HTTP version
    * will be defaulted to 1.1.
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response(
        http_proto::status sc);

    /** Return a read-only view to the response
    */
    operator
    response_view() const noexcept
    {
        return response_view(ph_);
    }

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the reason string

        This field is obsolete in HTTP/1
        and should only be used for display
        purposes.
    */
    core::string_view
    reason() const noexcept
    {
        return core::string_view(
            ph_->cbuf + 13,
            ph_->prefix - 15);
    }

    /** Return the status code
    */
    http_proto::status
    status() const noexcept
    {
        return ph_->res.status;
    }

    /** Return the status code
    */
    unsigned short
    status_int() const noexcept
    {
        return ph_->res.status_int;
    }

    /** Return the HTTP version
    */
    http_proto::version
    version() const noexcept
    {
        return ph_->version;
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Set the version, status code of the response

        The reason phrase will be set to the
        standard text for the specified status
        code.

        @par sc The status code. This must not be
                @ref http_proto::status::unknown.

        @par v The HTTP-version.
    */
    void
    set_start_line(
        http_proto::status sc,
        http_proto::version v =
            http_proto::version::http_1_1)
    {
        set_impl(
            sc,
            static_cast<
                unsigned short>(sc),
            obsolete_reason(sc),
            v);
    }

    void
    set_start_line(
        unsigned short si,
        core::string_view reason,
        http_proto::version v)
    {
        set_impl(
            int_to_status(si),
            si,
            reason,
            v);
    }

    /** Swap this with another instance
    */
    void
    swap(response& other) noexcept
    {
        h_.swap(other.h_);
    }

    /** Swap two instances
    */
    // hidden friend
    friend
    void
    swap(
        response& t0,
        response& t1) noexcept
    {
        t0.swap(t1);
    }

private:
    BOOST_HTTP_PROTO_DECL
    void
    set_impl(
        http_proto::status sc,
        unsigned short si,
        core::string_view reason,
        http_proto::version v);
};

} // http_proto
} // boost

#endif
