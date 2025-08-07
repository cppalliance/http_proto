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

/** A modfiable container for HTTP requests.

    This container owns a request, represented by
    a buffer which is managed by performing
    dynamic memory allocations as needed. The
    contents may be inspected and modified, and
    the implementation maintains a useful
    invariant: changes to the request always leave
    it in a valid state.

    @par Example
    @code
    request req(method::get, "/");

    req.set(field::host, "example.com");
    req.set(field::accept_encoding, "gzip, deflate, br");
    req.set(field::cache_control, "no-cache");

    assert(req.buffer() ==
    "GET / HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "Cache-Control: no-cache\r\n"
    "\r\n");
    @endcode

    @see
        @ref static_request,
        @ref request_view.
*/
class request
    : public request_base
{
public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor.

        A default-constructed request contains
        a valid HTTP `GET` request with no headers.

        @par Example
        @code
        request req;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "GET / HTTP/1.1\r\n\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    BOOST_HTTP_PROTO_DECL
    request() noexcept;

    /** Constructor.

        Constructs a request from the string `s`,
        which must contain valid HTTP request
        or else an exception is thrown.
        The new request retains ownership by
        making a copy of the passed string.

        @par Example
        @code
        request req(
            "GET / HTTP/1.1\r\n"
            "Accept-Encoding: gzip, deflate, br\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n");
        @endcode

        @par Postconditions
        @code
        this->buffer.data() != s.data()
        @endcode

        @par Complexity
        Linear in `s.size()`.

        @par Exception Safety
        Calls to allocate may throw.
        Exception thrown on invalid input.

        @throw system_error
        The input does not contain a valid request.

        @param s The string to parse.
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    request(
        core::string_view s);

    /** Constructor.

        The start-line of the request will
        contain the standard text for the
        supplied method, target and HTTP version.

        @par Example
        @code
        request req(method::get, "/index.html", version::http_1_0);
        @endcode

        @par Complexity
        Linear in `to_string(m).size() + t.size()`.

        @par Exception Safety
        Calls to allocate may throw.

        @param m The method to set.

        @param t The string representing a target.

        @param v The version to set.
    */
    request(
        http_proto::method m,
        core::string_view t,
        http_proto::version v) noexcept
        : request()
    {
        set_start_line(m, t, v);
    }

    /** Constructor.

        The start-line of the request will
        contain the standard text for the
        supplied method and target with the HTTP
        version defaulted to `HTTP/1.1`.

        @par Example
        @code
        request req(method::get, "/index.html");
        @endcode

        @par Complexity
        Linear in `obsolete_reason(s).size()`.

        @par Exception Safety
        Calls to allocate may throw.

        @param m The method to set.

        @param t The string representing a target.
    */
    request(
        http_proto::method m,
        core::string_view t)
        : request(
            m, t, http_proto::version::http_1_1)
    {
    }

    /** Constructor.

        Allocates `cap` bytes initially, with an
        upper limit of `max_cap`. Growing beyond
        `max_cap` will throw an exception.

        Useful when an estimated initial size is
        known, but further growth up to a maximum
        is allowed.

        When `cap == max_cap`, the container
        guarantees to never allocate.

        @par Preconditions
        @code
        max_cap >= cap
        @endcode

        @par Exception Safety
        Calls to allocate may throw.

        @param cap Initial capacity in bytes (may be `0`).

        @param max_cap Maximum allowed capacity in bytes.
    */
    BOOST_HTTP_PROTO_DECL
    request(
        std::size_t cap,
        std::size_t max_cap = std::size_t(-1));

    /** Constructor.

        The contents of `r` are transferred
        to the newly constructed object,
        which includes the underlying
        character buffer.
        After construction, the moved-from
        object is as if default-constructed.

        @par Postconditions
        @code
        r.buffer() == "GET / HTTP/1.1\r\n\r\n"
        @endcode

        @par Complexity
        Constant.

        @param r The request to move from.
    */
    BOOST_HTTP_PROTO_DECL
    request(request&& r) noexcept;

    /** Constructor.

        The newly constructed object contains
        a copy of `r`.

        @par Postconditions
        @code
        this->buffer() == r.buffer() && this->buffer.data() != r.buffer().data()
        @endcode

        @par Complexity
        Linear in `r.size()`.

        @par Exception Safety
        Calls to allocate may throw.

        @param r The request to copy.
    */
    BOOST_HTTP_PROTO_DECL
    request(request const& r);

    /** Constructor.

        The newly constructed object contains
        a copy of `r`.

        @par Postconditions
        @code
        this->buffer() == r.buffer() && this->buffer.data() != r.buffer().data()
        @endcode

        @par Complexity
        Linear in `r.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.

        @param r The request to copy.
    */
    BOOST_HTTP_PROTO_DECL
    request(
        request_view const& r);

    /** Assignment

        The contents of `r` are transferred to
        `this`, including the underlying
        character buffer. The previous contents
        of `this` are destroyed.
        After assignment, the moved-from
        object is as if default-constructed.

        @par Postconditions
        @code
        r.buffer() == "GET / HTTP/1.1\r\n\r\n"
        @endcode

        @par Complexity
        Constant.

        @param r The request to assign from.

        @return A reference to this object.
    */
    BOOST_HTTP_PROTO_DECL
    request&
    operator=(request&& r) noexcept;


    /** Assignment.

        The contents of `r` are copied and
        the previous contents of `this` are
        discarded.

        @par Postconditions
        @code
        this->buffer() == r.buffer() && this->buffer().data() != r.buffer().data()
        @endcode

        @par Complexity
        Linear in `r.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param r The request to copy.

        @return A reference to this object.
    */
    request&
    operator=(
        request const& r)
    {
        copy_impl(*r.ph_);
        return *this;
    }

    /** Assignment.

        The contents of `r` are copied and
        the previous contents of `this` are
        discarded.

        @par Postconditions
        @code
        this->buffer() == r.buffer() && this->buffer().data() != r.buffer().data()
        @endcode

        @par Complexity
        Linear in `r.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param r The request to copy.

        @return A reference to this object.
    */
    request&
    operator=(
        request_view const& r)
    {
        copy_impl(*r.ph_);
        return *this;
    }

    //--------------------------------------------

    /** Swap.

        Exchanges the contents of this request
        with another request. All views,
        iterators and references remain valid.

        If `this == &other`, this function call has no effect.

        @par Example
        @code
        request r1(method::get, "/");
        request r2(method::delete_, "/item/42");
        r1.swap(r2);
        assert(r1.buffer() == "DELETE /item/42 HTTP/1.1\r\n\r\n" );
        assert(r2.buffer() == "GET / HTTP/1.1\r\n\r\n" );
        @endcode

        @par Complexity
        Constant

        @param other The object to swap with
    */
    void
    swap(request& other) noexcept
    {
        h_.swap(other.h_);
        std::swap(max_cap_, other.max_cap_);
    }


    /** Swap.

        Exchanges the contents of `v0` with
        another `v1`. All views, iterators and
        references remain valid.

        If `&v0 == &v1`, this function call has no effect.

        @par Example
        @code
        request r1(method::get, "/");
        request r2(method::delete_, "/item/42");
        std::swap(r1, r2);
        assert(r1.buffer() == "DELETE /item/42 HTTP/1.1\r\n\r\n" );
        assert(r2.buffer() == "GET / HTTP/1.1\r\n\r\n" );
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
            @ref request::swap
    */
    friend
    void
    swap(
        request& v0,
        request& v1) noexcept
    {
        v0.swap(v1);
    }
};

} // http_proto
} // boost

#endif
