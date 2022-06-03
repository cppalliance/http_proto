//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_HPP
#define BOOST_HTTP_PROTO_RESPONSE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/version.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class response_view;
#endif

/** Container for HTTP responses
*/
class BOOST_SYMBOL_VISIBLE
    response
    : public fields_base
{
    BOOST_HTTP_PROTO_DECL
    void
    set_start_line_impl(
        http_proto::status sc,
        unsigned short si,
        string_view reason,
        http_proto::version v);

public:
    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response(
        http_proto::status sc,
        http_proto::version v =
            http_proto::version::http_1_1);

    //--------------------------------------------

    /** Constructor
    */
    response() noexcept
        : fields_base(
            detail::kind::response)
    {
    }

    /** Constructor

        The moved-from object will be
        left in the default-constructed
        state.
    */
    BOOST_HTTP_PROTO_DECL
    response(response&&) noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response(response const&);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    response&
    operator=(response&&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    response&
    operator=(response const&);

    //--------------------------------------------

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response(
        response_view const&);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    response&
    operator=(response_view const&);

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the reason string
    */
    string_view
    reason() const noexcept
    {
        return string_view(
            h_.cbuf + 13,
            h_.prefix - 15);
    }

    /** Return the status code
    */
    http_proto::status
    status() const noexcept
    {
        return h_.res.status;
    }

    /** Return the status code
    */
    unsigned short
    status_int() const noexcept
    {
        return h_.res.status_int;
    }

    /** Return the HTTP version
    */
    http_proto::version
    version() const noexcept
    {
        return h_.version;
    }

    /** Return a read-only view to the response
    */
    BOOST_HTTP_PROTO_DECL
    operator
    response_view() const noexcept;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Clear the contents, but not the capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

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
        set_start_line_impl(
            sc,
            static_cast<
                unsigned short>(sc),
            obsolete_reason(sc),
            v);
    }

    void
    set_start_line(
        unsigned short si,
        string_view reason,
        http_proto::version v)
    {
        set_start_line_impl(
            int_to_status(si),
            si,
            reason,
            v);
    }

    /** Swap this with another instance
    */
    BOOST_HTTP_PROTO_DECL
    void
    swap(response& other) noexcept;

    /** Swap two instances
    */
    friend
    void
    swap(
        response& v1,
        response& v2) noexcept
    {
        v1.swap(v2);
    }
};

} // http_proto
} // boost

#endif
