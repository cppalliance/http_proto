//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP
#define BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** A view to a valid HTTP response.

    Objects of this type represent a view to
    a HTTP response container. That is, it acts
    like a `core::string_view` in terms of
    ownership. The caller is responsible for
    ensuring that the lifetime of the underlying
    buffer extends until it is no
    longer referenced.

    @see
        @ref response,
        @ref static_response,
        @ref response_parser,
*/
class response_view
    : public message_view_base
{
    friend class response_base;
    friend class response_parser;

    explicit
    response_view(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
        BOOST_ASSERT(ph_->kind ==
            detail::kind::response);
    }

public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor.

        A default-constructed response view refer
        to a valid HTTP 200 OK response with no
        headers, which always remains valid.

        @par Example
        @code
        response_view resv;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "HTTP/1.1 200 OK\r\n\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    response_view() noexcept
        : fields_view_base(
            detail::header::get_default(
                detail::kind::response))
    {
    }

    /** Constructor.

        After construction, both response views
        reference the same underlying buffer.
        Ownership is not transferred.

        @par Postconditions
        @code
        this->buffer().data() == other.buffer().data()
        @endcode

        @par Complexity
        Constant.

        @param other The other view.
    */
    response_view(
        response_view const& other) noexcept = default;

    /** Assignment.

        After assignment, both response views
        reference the same underlying buffer.
        Ownership is not transferred.

        @par Postconditions
        @code
        this->buffer().data() == other.buffer().data()
        @endcode

        @par Complexity
        Constant.

        @param other The other view.
        @return A reference to this object.
    */
    response_view&
    operator=(
        response_view const& other) noexcept = default;

    /** Destructor

        Any reference, iterators, or other views
        which reference the same underlying
        buffer remain valid.
    */
    ~response_view() = default;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the reason string

        This field is obsolete in `HTTP/1.1`
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

    /** Return the status code.
    */
    http_proto::status
    status() const noexcept
    {
        return ph_->res.status;
    }

    /** Return the status code integer.
    */
    unsigned short
    status_int() const noexcept
    {
        return ph_->res.status_int;
    }

    //--------------------------------------------

    /** Swap.

        Exchanges the view with that of `other`.
        All iterators and references remain valid.

        If `this == &other`, this function call has no effect.

        @par Example
        @code
        response r1(status::ok);
        response r2(status::bad_request);
        response_view v1 = r1;
        response_view v2 = r2;
        v1.swap(v2);
        assert(v1.buffer() == "HTTP/1.1 400 Bad Request\r\n\r\n" );
        assert(v2.buffer() == "HTTP/1.1 200 OK\r\n\r\n" );
        @endcode

        @par Complexity
        Constant.

        @param other The object to swap with.
    */
    void
    swap(response_view& other) noexcept
    {
        auto ph = ph_;
        ph_ = other.ph_;
        ph_ = ph;
    }

    /** Swap.

        Exchanges the view of `v0` with
        another `v1`. All iterators and
        references remain valid.

        If `&v0 == &v1`, this function call has no effect.

        @par Example
        @code
        response r1(status::ok);
        response r2(status::bad_request);
        response_view v1 = r1;
        response_view v2 = r2;
        std::swap(v1, v2);
        assert(v1.buffer() == "HTTP/1.1 400 Bad Request\r\n\r\n" );
        assert(v2.buffer() == "HTTP/1.1 200 OK\r\n\r\n" );
        @endcode

        @par Effects
        @code
        v0.swap(v1);
        @endcode

        @par Complexity
        Constant.

        @param v0 The first object to swap.
        @param v1 The second object to swap.

        @see
            @ref response_view::swap.
    */
    friend
    void
    swap(
        response_view& v0,
        response_view& v1) noexcept
    {
        v0.swap(v1);
    }
};

} // http_proto
} // boost

#endif
