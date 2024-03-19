//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_HPP
#define BOOST_HTTP_PROTO_REQUEST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/request_view.hpp>

namespace boost {
namespace http_proto {

/** Container for HTTP requests
*/
class request final
    : public message_base
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
    operator=(request const& other)
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

    /** Return a read-only view to the request
    */
    operator
    request_view() const noexcept
    {
        return request_view(ph_);
    }

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the method as an integral constant

        If the method returned is equal to
        @ref method::unknown, the method may
        be obtained as a string instead, by
        calling @ref method_text.
    */
    http_proto::method
    method() const noexcept
    {
        return ph_->req.method;
    }

    /** Return the method as a string
    */
    core::string_view
    method_text() const noexcept
    {
        return core::string_view(
            ph_->cbuf,
            ph_->req.method_len);
    }

    /** Return the request-target string
    */
    core::string_view
    target() const noexcept
    {
        return core::string_view(
            ph_->cbuf +
                ph_->req.method_len + 1,
            ph_->req.target_len);
    }

    /** Return the HTTP-version
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

    /** Set the method of the request to the enum
    */
    void
    set_method(
        http_proto::method m)
    {
        set_impl(
            m,
            to_string(m),
            target(),
            version());
    }

    /** Set the method of the request to the string
    */
    void
    set_method(
        core::string_view s)
    {
        set_impl(
            string_to_method(s),
            s,
            target(),
            version());
    }

    /** Set the target string of the request

        This function sets the request-target.
        The caller is responsible for ensuring
        that the string passed is syntactically
        valid.
    */
    void
    set_target(
        core::string_view s)
    {
        set_impl(
            ph_->req.method,
            method_text(),
            s,
            version());
    }

    /** Set the HTTP version of the request
    */
    void
    set_version(
        http_proto::version v)
    {
        set_impl(
            ph_->req.method,
            method_text(),
            target(),
            v);
    }

    /** Set the method, target, and version of the request

        This is more efficient than setting the
        properties individually.
    */
    void
    set_start_line(
        http_proto::method m,
        core::string_view t,
        http_proto::version v)
    {
        set_impl(m, to_string(m), t, v);
    }

    /** Set the method, target, and version of the request

        This is more efficient than setting the
        properties individually.
    */
    void
    set_start_line(
        core::string_view m,
        core::string_view t,
        http_proto::version v)
    {
        set_impl(string_to_method(m), m, t, v);
    }

    /** Set the Expect header
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_expect_100_continue(bool b);

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

private:
    BOOST_HTTP_PROTO_DECL
    void
    set_impl(
        http_proto::method m,
        core::string_view ms,
        core::string_view t,
        http_proto::version v);
};

} // http_proto
} // boost

#endif
