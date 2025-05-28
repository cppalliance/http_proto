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

/** A modfiable static container for HTTP responses
*/
template<std::size_t Capacity>
class static_response
    : public response_base
{
    alignas(entry)
    char buf_[Capacity];

public:
    /** Constructor
    */
    static_response() noexcept
        : fields_view_base(&this->fields_base::h_)
        , response_base(buf_, Capacity)
    {
    }

    /** Constructor
    */
    explicit
    static_response(
        core::string_view s)
        : fields_view_base(&this->fields_base::h_)
        , response_base(s, buf_, Capacity)
    {
    }

    /** Constructor
    */
    static_response(
        http_proto::status sc,
        http_proto::version v)
        : static_response()
    {
        if( sc != h_.res.status ||
            v != h_.version)
            set_start_line(sc, v);
    }

    /** Constructor
    *
    * The start-line of the response will contain the standard
    * text for the supplied status code and the HTTP version
    * will be defaulted to 1.1.
    */
    explicit
    static_response(
        http_proto::status sc)
        : static_response(
            sc, http_proto::version::http_1_1)
    {
    }

    /** Constructor
    */
    static_response(
        static_response const& other) noexcept
        : fields_view_base(&this->fields_base::h_)
        , response_base(*other.ph_, buf_, Capacity)
    {
    }

    /** Constructor
    */
    static_response(
        response_view const& other)
        : fields_view_base(&this->fields_base::h_)
        , response_base(*other.ph_, buf_, Capacity)
    {
    }

    /** Assignment
    */
    static_response&
    operator=(
        static_response const& other) noexcept
    {
        copy_impl(*other.ph_);
        return *this;
    }

    /** Assignment
    */
    static_response&
    operator=(
        response_view const& other)
    {
        copy_impl(*other.ph_);
        return *this;
    }
};

} // http_proto
} // boost

#endif
