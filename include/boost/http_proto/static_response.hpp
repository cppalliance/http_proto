//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STATIC_RESPONSE_HPP
#define BOOST_HTTP_PROTO_STATIC_RESPONSE_HPP

#include <boost/http_proto/response_base.hpp>

namespace boost {
namespace http_proto {

/** A modifiable static container for HTTP responses.

    This container owns a response, represented
    by an inline buffer with fixed capacity.
    The contents may be inspected and modified,
    and the implementation maintains a useful
    invariant: changes to the response always
    leave it in a valid state.

    @par Example
    @code
    static_response<1024> res(status::not_found);

    res.set(field::server, "Boost.HttpProto");
    res.set(field::content_type, "text/plain");
    res.set_content_length(80);

    assert(res.buffer() ==
        "HTTP/1.1 404 Not Found\r\n"
        "Server: Boost.HttpProto\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 80\r\n"
        "\r\n");
    @endcode

    @par Invariants
    @code
    this->capacity_in_bytes() == Capacity && this->max_capacity_in_bytes() == Capacity 
    @endcode

    @tparam Capacity The maximum capacity in bytes.

    @see
        @ref response,
        @ref response_view.
*/
template<std::size_t Capacity>
class static_response
    : public response_base
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

        A default-constructed response contains
        a valid HTTP 200 OK response with no headers.

        @par Example
        @code
        static_response<1024> res;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "HTTP/1.1 200 OK\r\n\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    static_response() noexcept
        : fields_view_base(&this->fields_base::h_)
        , response_base(buf_, Capacity)
    {
    }

    /** Constructor.

        Constructs a response from the string `s`,
        which must contain valid HTTP response
        or else an exception is thrown.
        The new response retains ownership by
        making a copy of the passed string.

        @par Example
        @code
        static_response<1024> res(
            "HTTP/1.1 404 Not Found\r\n"
            "Server: Boost.HttpProto\r\n"
            "Content-Type: text/plain\r\n"
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
        The input does not contain a valid response.

        @throw std::length_error
        Max capacity would be exceeded.

        @param s The string to parse.
    */
    explicit
    static_response(
        core::string_view s)
        : fields_view_base(&this->fields_base::h_)
        , response_base(s, buf_, Capacity)
    {
    }

    /** Constructor.

        The start-line of the response will
        contain the standard text for the
        supplied status code and HTTP version.

        @par Example
        @code
        static_response<1024> res(status::not_found, version::http_1_0);
        @endcode

        @par Complexity
        Linear in `obsolete_reason(s).size()`.

        @par Exception Safety
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param sc The status code.

        @param v The HTTP version.
    */
    static_response(
        http_proto::status sc,
        http_proto::version v)
        : static_response()
    {
        set_start_line(sc, v);
    }

    /** Constructor.

        The start-line of the response will
        contain the standard text for the
        supplied status code with the HTTP version
        defaulted to `HTTP/1.1`.

        @par Example
        @code
        static_response<1024> res(status::not_found);
        @endcode

        @par Complexity
        Linear in `obsolete_reason(s).size()`.

        @par Exception Safety
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param sc The status code.
    */
    explicit
    static_response(
        http_proto::status sc)
        : static_response(
            sc, http_proto::version::http_1_1)
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

        @param r The response to copy.
    */
    static_response(
        static_response const& r) noexcept
        : fields_view_base(&this->fields_base::h_)
        , response_base(*r.ph_, buf_, Capacity)
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

        @param r The response to copy.
    */
    static_response(
        response_view const& r)
        : fields_view_base(&this->fields_base::h_)
        , response_base(*r.ph_, buf_, Capacity)
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

        @param r The response to copy.

        @return A reference to this object.
    */
    static_response&
    operator=(
        static_response const& r) noexcept
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

        @param r The response to copy.

        @return A reference to this object.
    */
    static_response&
    operator=(
        response_view const& r)
    {
        copy_impl(*r.ph_);
        return *this;
    }
};

} // http_proto
} // boost

#endif
