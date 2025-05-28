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

/** A modfiable static container for HTTP requests
*/
template<std::size_t Capacity>
class static_request
    : public request_base
{
    alignas(entry)
    char buf_[Capacity];

public:
    /** Constructor
    */
    static_request() noexcept
        : fields_view_base(&this->fields_base::h_)
        , request_base(buf_, Capacity)
    {
    }

    /** Constructor
    */
    explicit
    static_request(
        core::string_view s)
        : fields_view_base(&this->fields_base::h_)
        , request_base(s, buf_, Capacity)
    {
    }

    /** Constructor
    */
    static_request(
        static_request const& other) noexcept
        : fields_view_base(&this->fields_base::h_)
        , request_base(*other.ph_, buf_, Capacity)
    {
    }

    /** Constructor
    */
    static_request(
        request_view const& other)
        : fields_view_base(&this->fields_base::h_)
        , request_base(*other.ph_, buf_, Capacity)
    {
    }

    /** Assignment
    */
    static_request&
    operator=(
        static_request const& other) noexcept
    {
        copy_impl(*other.ph_);
        return *this;
    }

    /** Assignment
    */
    static_request&
    operator=(
        request_view const& other)
    {
        copy_impl(*other.ph_);
        return *this;
    }
};

} // http_proto
} // boost

#endif
