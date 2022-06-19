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

namespace boost {
namespace http_proto {

/** Holds a buffer sequence that cannot be modified
*/
class mutable_buffers
{
    std::size_t n_ = 0;
    asio::mutable_buffer v_[8];

public:
    mutable_buffers() = default;

    mutable_buffers(
        asio::mutable_buffer const* data,
        std::size_t size) noexcept
        : n_(size)
    {
        for(std::size_t i = 0;
            i < n_; ++i)
            v_[i] = data[i];
    }

    asio::mutable_buffer const*
    begin() const noexcept
    {
        return &v_[0];
    }

    asio::mutable_buffer const*
    end() const noexcept
    {
        return begin() + n_;
    }
};

//------------------------------------------------

/** Holds a buffer sequence that cannot be modified
*/
class const_buffers
{
    std::size_t n_ = 0;
    asio::const_buffer v_[8];

public:
    const_buffers() = default;

    const_buffers(
        asio::const_buffer const* data,
        std::size_t size) noexcept
        : n_(size)
    {
        for(std::size_t i = 0;
            i < n_; ++i)
            v_[i] = data[i];
    }

    const_buffers(
        asio::const_buffer const* data1,
        std::size_t size1,
        asio::const_buffer const* data2,
        std::size_t size2) noexcept
        : n_(size1 + size2)
    {
        std::size_t i = 0;
        for(;i < size1; ++i)
            v_[i] = data1[i];
        size2 += size1;
        for(;i < size2; ++i)
            v_[i] = data2[i - size1];
    }

    asio::const_buffer const*
    begin() const noexcept
    {
        return &v_[0];
    }

    asio::const_buffer const*
    end() const noexcept
    {
        return begin() + n_;
    }

    friend
    const_buffers
    operator+(
        const_buffers b1,
        const_buffers b2)
    {
        return const_buffers(
            b1.begin(), b1.n_,
            b2.begin(), b2.n_);
    }
};

} // http_proto
} // boost

#endif
