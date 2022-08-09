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
#include <cstdlib>

namespace boost {
namespace http_proto {

//------------------------------------------------

class mutable_buffer
{
    void* p_ = nullptr;
    std::size_t n_ = 0;

public:
    mutable_buffer() = default;
    mutable_buffer(
        mutable_buffer const&) = default;
    mutable_buffer& operator=(
        mutable_buffer const&) = default;

    mutable_buffer(
        void* data,
        std::size_t size) noexcept
        : p_(data)
        , n_(size)
    {
    }

    void*
    data() const noexcept
    {
        return p_;
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    mutable_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = static_cast<
                char*>(p_) + n_;
            n_ = 0;
            return *this;
        }
        p_ = static_cast<
            char*>(p_) + n;
        n_ -= n;
        return *this;
    }
};

//------------------------------------------------

class const_buffer
{
    void const* p_ = nullptr;
    std::size_t n_ = 0;

public:
    const_buffer() = default;
    const_buffer(
        const_buffer const&) = default;
    const_buffer& operator=(
        const_buffer const&) = default;

    const_buffer(
        void const* data,
        std::size_t size) noexcept
        : p_(data)
        , n_(size)
    {
    }

    const_buffer(
        mutable_buffer const& other) noexcept
        : p_(other.data())
        , n_(other.size())
    {
    }

    void const*
    data() const noexcept
    {
        return p_;
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    const_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = static_cast<
                char const*>(p_) + n_;
            n_ = 0;
            return *this;
        }
        p_ = static_cast<
            char const*>(p_) + n;
        n_ -= n;
        return *this;
    }
};

//------------------------------------------------

class const_buffers
{
    const_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

public:
    using value_type = const_buffer;

    using iterator = value_type const*;

    const_buffers() = default;

    const_buffers(
        const_buffers const&) = default;

    const_buffers& operator=(
        const_buffers const&) = default;

    const_buffers(
        value_type const* p,
        std::size_t n) noexcept
        : p_(p)
        , n_(n)
    {
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    iterator
    begin() const noexcept
    {
        return p_;
    }

    iterator
    end() const noexcept
    {
        return p_ + n_;
    }
};

//------------------------------------------------

class mutable_buffers
{
    mutable_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

public:
    using value_type = mutable_buffer;

    using iterator = value_type const*;

    mutable_buffers() = default;

    mutable_buffers(
        mutable_buffers const&) = default;

    mutable_buffers& operator=(
        mutable_buffers const&) = default;

    mutable_buffers(
        value_type const* p,
        std::size_t n) noexcept
        : p_(p)
        , n_(n)
    {
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    iterator
    begin() const noexcept
    {
        return p_;
    }

    iterator
    end() const noexcept
    {
        return p_ + n_;
    }
};

} // http_proto
} // boost

#endif
