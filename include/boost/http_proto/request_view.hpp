//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_VIEW_HPP
#define BOOST_HTTP_PROTO_REQUEST_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/headers_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
enum class method : char;
enum class version : char;
#endif

class request_view
{
    using off_t = std::uint16_t;

    friend class request_parser;

    off_t method_len_ = 0;
    off_t target_len_ = 0;
    http_proto::method method_;
    http_proto::version version_;

    BOOST_HTTP_PROTO_DECL
    request_view(
        char const* buf,
        std::size_t count,
        std::size_t capacity,
        std::size_t fields_bytes,
        std::size_t prefix_bytes,
        std::size_t method_len,
        std::size_t target_len,
        http_proto::method method,
        http_proto::version version) noexcept;

public:
    /** The field values for this message
    */
    headers_view fields;

    request_view(
        request_view const&) = default;
    request_view& operator=(
        request_view const&) = default;

    /** Constructor

        The contents of default constructed
        messages are undefined. The only valid
        operations are assignment and destruction.
    */
    BOOST_HTTP_PROTO_DECL
    request_view() noexcept;

    http_proto::method
    method() const noexcept
    {
        return method_;
    };

    string_view
    method_str() const noexcept
    {
        return string_view(
            fields.base(),
            method_len_);
    }

    string_view
    target() const noexcept
    {
        return string_view(
            fields.base() +
                method_len_ + 1,
            target_len_);
    }

    http_proto::version
    version() const noexcept
    {
        return version_;
    }
};

} // http_proto
} // boost

#endif
