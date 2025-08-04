//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_VIEW_HPP
#define BOOST_HTTP_PROTO_REQUEST_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** A view to a valid HTTP request.

    Objects of this type represent a view to
    a HTTP request container. That is, it acts
    like a `core::string_view` in terms of
    ownership. The caller is responsible for
    ensuring that the lifetime of the underlying
    buffer extends until it is no
    longer referenced.

    @see
        @ref request,
        @ref static_request,
        @ref request_parser,
*/
class request_view
    : public message_view_base
{
    friend class request_base;
    friend class request_parser;

    explicit
    request_view(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
        BOOST_ASSERT(ph_->kind ==
            detail::kind::request);
    }

public:
    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor.

        A default-constructed request views refer
        to a valid HTTP `GET` request with no
        headers, which always remains valid.

        @par Example
        @code
        request_view reqv;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "GET / HTTP/1.1\r\n\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    request_view() noexcept
        : fields_view_base(
            detail::header::get_default(
                detail::kind::request))
    {
    }

    /** Constructor.

        After construction, both request views
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
    request_view(
        request_view const& other) noexcept = default;

    /** Assignment.

        After assignment, both request views
        reference the same underlying buffer.
        Ownership is not transferred.

        @par Postconditions
        @code
        this->buffer().data() == other.buffer().data()
        @endcode

        @par Complexity
        Constant.

        @return A reference to this object.

        @param other The other view.
    */
    request_view&
    operator=(
        request_view const& other) noexcept = default;

    /** Destructor

        Any reference, iterators, or other views
        which reference the same underlying
        buffer remain valid.
    */
    ~request_view() = default;

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
    };

    /** Return the method as a string
    */
    core::string_view
    method_text() const noexcept
    {
        return core::string_view(
            ph_->cbuf,
            ph_->req.method_len);
    }

    /** Return the request-target string.
    */
    core::string_view
    target() const noexcept
    {
        return core::string_view(
            ph_->cbuf +
                ph_->req.method_len + 1,
            ph_->req.target_len);
    }

    //--------------------------------------------

    /** Swap.

        Exchanges the view with that of `other`.
        All iterators and references remain valid.

        If `this == &other`, this function call has no effect.

        @par Example
        @code
        request r1(method::get, "/");
        request r2(method::delete_, "/item/42");
        request_view v1 = r1;
        request_view v2 = r2;
        v1.swap(v2);
        assert(v1.buffer() == "DELETE /item/42 HTTP/1.1\r\n\r\n" );
        assert(v2.buffer() == "GET / HTTP/1.1\r\n\r\n" );
        @endcode

        @par Complexity
        Constant.

        @param other The object to swap with.
    */
    void
    swap(request_view& other) noexcept
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
        request r1(method::get, "/");
        request r2(method::delete_, "/item/42");
        request_view v1 = r1;
        request_view v2 = r2;
        std::swap(v1, v2);
        assert(v1.buffer() == "DELETE /item/42 HTTP/1.1\r\n\r\n" );
        assert(v2.buffer() == "GET / HTTP/1.1\r\n\r\n" );
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
            @ref request_view::swap.
    */
    friend
    void
    swap(
        request_view& v0,
        request_view& v1) noexcept
    {
        v0.swap(v1);
    }
};

} // http_proto
} // boost

#endif
