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
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response(
        std::size_t initial_size);

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response(
        std::size_t initial_size,
        std::size_t max_capacity);

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
