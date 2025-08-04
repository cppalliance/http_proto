//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STATIC_REQUEST_HPP
#define BOOST_HTTP_PROTO_STATIC_REQUEST_HPP

#include <boost/http_proto/request_base.hpp>

namespace boost {
namespace http_proto {

/** A modfiable static container for HTTP requests.

    This container owns a request, represented
    by an inline buffer with fixed capacity.
    The contents may be inspected and modified,
    and the implementation maintains a useful
    invariant: changes to the request always
    leave it in a valid state.

    @par Example
    @code
    static_request<1024> req(method::get, "/");

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

    @par Invariants
    @code
    this->capacity_in_bytes() == Capacity && this->max_capacity_in_bytes() == Capacity 
    @endcode

    @tparam Capacity The maximum capacity in bytes.

    @see
        @ref request,
        @ref request_view.
*/
template<std::size_t Capacity>
class static_request
    : public request_base
{
    alignas(entry)
    char buf_[Capacity];

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
        static_request<1024> req;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "GET / HTTP/1.1\r\n\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    static_request() noexcept
        : fields_view_base(&this->fields_base::h_)
        , request_base(buf_, Capacity)
    {
    }

    /** Constructor.

        Constructs a request from the string `s`,
        which must contain valid HTTP request
        or else an exception is thrown.
        The new request retains ownership by
        making a copy of the passed string.

        @par Example
        @code
        static_request<1024> req(
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
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        The input does not contain a valid request.

        @throw std::length_error
        Max capacity would be exceeded.

        @param s The string to parse.
    */
    explicit
    static_request(
        core::string_view s)
        : fields_view_base(&this->fields_base::h_)
        , request_base(s, buf_, Capacity)
    {
    }

    /** Constructor.

        The start-line of the request will
        contain the standard text for the
        supplied method, target and HTTP version.

        @par Example
        @code
        static_request<1024> req(method::get, "/index.html", version::http_1_0);
        @endcode

        @par Complexity
        Linear in `to_string(m).size() + t.size()`.

        @par Exception Safety
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param m The method to set.

        @param t The string representing a target.

        @param v The version to set.
    */
    static_request(
        http_proto::method m,
        core::string_view t,
        http_proto::version v) noexcept
        : static_request()
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
        static_request<1024> req(method::get, "/index.html");
        @endcode

        @par Complexity
        Linear in `obsolete_reason(s).size()`.

        @par Exception Safety
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param m The method to set.

        @param t The string representing a target.
    */
    static_request(
        http_proto::method m,
        core::string_view t)
        : static_request(
            m, t, http_proto::version::http_1_1)
    {
    }

    /** Constructor.

        The newly constructed object contains
        a copy of `r`.

        @par Postconditions
        @code
        this->buffer() == r.buffer() && this->buffer.data() != r.buffer().data()
        @endcode

        @par Complexity
        Linear in `r.size()`.

        @param r The request to copy.
    */
    static_request(
        static_request const& r) noexcept
        : fields_view_base(&this->fields_base::h_)
        , request_base(*r.ph_, buf_, Capacity)
    {
    }

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
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param r The request to copy.
    */
    static_request(
        request_view const& r)
        : fields_view_base(&this->fields_base::h_)
        , request_base(*r.ph_, buf_, Capacity)
    {
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

        @param r The request to copy.

        @return A reference to this object.
    */
    static_request&
    operator=(
        static_request const& r) noexcept
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
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param r The request to copy.

        @return A reference to this object.
    */
    static_request&
    operator=(
        request_view const& r)
    {
        copy_impl(*r.ph_);
        return *this;
    }
};

} // http_proto
} // boost

#endif
