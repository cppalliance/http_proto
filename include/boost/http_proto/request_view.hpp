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
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

class request_view
{
    friend class request_parser;

    char const* base_;
    std::size_t size_;
    http_proto::method method_;
    int version_;
    unsigned short n_method_;
    unsigned short n_target_;

    request_view(
        char const* base,
        std::size_t size,
        unsigned short n_method,
        unsigned short n_target,
        int version) noexcept;

public:
    request_view(
        request_view const&) = default;

    request_view& operator=(
        request_view const&) = default;

    BOOST_HTTP_PROTO_DECL
    request_view() noexcept;

    char const*
    data() const noexcept
    {
        return base_;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    http_proto::method
    method() const noexcept
    {
        return method_;
    };

    string_view
    method_str() const noexcept
    {
        return {
            base_, n_method_ };
    }

    string_view
    target() const noexcept
    {
        return {
            base_ + n_method_ + 1,
            n_target_ };
    }

    int
    version() const noexcept
    {
        return version_;
    }
};

} // http_proto
} // boost

#endif
