//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STRING_BODY_HPP
#define BOOST_HTTP_PROTO_STRING_BODY_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/string_view.hpp>
#include <string>
#include <utility>

namespace boost {
namespace http_proto {

class string_body
{
    std::string s_;
    const_buffer cb_;

public:
    using iterator = const_buffer const*;

    string_body(
        string_body&&) = default;
    string_body(
        string_body const&) = delete;

    string_body(
        std::string s) noexcept
        : s_(std::move(s))
        , cb_(s_.data(), s_.size())
    {
    }

    iterator
    begin() const noexcept
    {
        return &cb_;
    }

    iterator
    end() const noexcept
    {
        return (&cb_) + 1;
    }
};

} // http_proto
} // boost

#endif
