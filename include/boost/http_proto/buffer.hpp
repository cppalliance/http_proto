//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BUFFER_HPP
#define BOOST_HTTP_PROTO_BUFFER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/asio/buffer.hpp>
#include <cstdlib>
#include <type_traits>

namespace boost {
namespace http_proto {

/** Holds a buffer sequence that cannot be modified
*/
class mutable_buffers
{
    asio::mutable_buffer const* begin_ = nullptr;
    asio::mutable_buffer const* end_ = nullptr;

public:
    mutable_buffers() = default;

    mutable_buffers(
        asio::mutable_buffer const* data,
        std::size_t size) noexcept
        : begin_(data)
        , end_(data + size)
    {
    }

    asio::mutable_buffer const*
    begin() const noexcept
    {
        return begin_;
    }

    asio::mutable_buffer const*
    end() const noexcept
    {
        return end_;
    }
};

//------------------------------------------------

/** Holds a buffer sequence that cannot be modified
*/
class const_buffers
{
    asio::const_buffer const* begin_ = nullptr;
    asio::const_buffer const* end_ = nullptr;

public:
    const_buffers() = default;

    const_buffers(
        asio::const_buffer const* data,
        std::size_t size) noexcept
        : begin_(data)
        , end_(data + size)
    {
    }

    asio::const_buffer const*
    begin() const noexcept
    {
        return begin_;
    }

    asio::const_buffer const*
    end() const noexcept
    {
        return end_;
    }
};

} // http_proto
} // boost

#endif
