//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STRING_BODY_HPP
#define BOOST_HTTP_PROTO_STRING_BODY_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <string>
#include <utility>

namespace boost {
namespace http_proto {

class string_body
{
    std::string s_;
    buffers::const_buffer cb_;

public:
    using value_type = buffers::const_buffer;
    using const_iterator = buffers::const_buffer const*;

    string_body(
        string_body&& other) noexcept
        : s_(std::move(other.s_))
        , cb_(s_.data(), s_.size())
    {
        other.cb_ = {};
    }

    string_body(
        string_body const& other) = delete;

    string_body(
        std::string s) noexcept
        : s_(std::move(s))
        , cb_(s_.data(), s_.size())
    {
    }

    const_iterator
    begin() const noexcept
    {
        return &cb_;
    }

    const_iterator
    end() const noexcept
    {
        return &cb_ + 1;
    }
};

//------------------------------------------------

} // http_proto
} // boost

#endif
