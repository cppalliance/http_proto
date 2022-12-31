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
#include <boost/config/workaround.hpp>
#include <cstdlib>
#include <type_traits>

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

#if BOOST_WORKAROUND(BOOST_MSVC, < 1910)
# pragma warning (push)
# pragma warning (disable: 4521) // multiple copy constructors specified
# pragma warning (disable: 4522) // multiple assignment operators specified
#endif

template<bool isConst>
class buffers_pair
{
public:
    // VFALCO: This type is public otherwise
    //         asio::buffers_iterator won't compile.
    using value_type = typename
        std::conditional<isConst,
            const_buffer,
            mutable_buffer>::type;

    using const_iterator = value_type const*;

    buffers_pair() = default;

#if defined(BOOST_HTTP_PROTO_DOCS) || ( \
        ! BOOST_WORKAROUND(BOOST_MSVC, < 1910))
    buffers_pair(
        buffers_pair const& other) = default;
    buffers_pair& operator=(
        buffers_pair const& other) = default;

#else
    buffers_pair(
        buffers_pair const& other)
        : buffers_pair(
            *other.begin(),
            *(other.begin() + 1))
    {
    }

    buffers_pair&
    operator=(buffers_pair const& other)
    {
        b_[0] = other.b_[0];
        b_[1] = other.b_[1];
        return *this;
    }
#endif

    // const pair construction
    // from mutable mutable pair
    template<
        bool isConst_ = isConst,
        class = typename std::enable_if<
            isConst_>::type>
    buffers_pair(
        buffers_pair<false> const& other)
        : buffers_pair(
            other.b_[0],
            other.b_[1])
    {
    }

    // const pair assignment
    // from mutable mutable pair
    template<
        bool isConst_ = isConst,
        class = typename std::enable_if<
            isConst_>::type>
    buffers_pair&
    operator=(
        buffers_pair<false> const& other)
    {
        b_[0] = other.b_[0];
        b_[1] = other.b_[1];
        return *this;
    }

    buffers_pair(
        value_type b0,
        value_type b1)
        : b_{b0, b1}
    {
    }

    const_iterator
    begin() const noexcept
    {
        return &b_[0];
    }

    const_iterator
    end() const noexcept
    {
        if(b_[1].size() > 0)
            return &b_[2];
        if(b_[0].size() > 0)
            return &b_[1];
        return begin();
    }

private:
    value_type b_[2];
};

#if BOOST_WORKAROUND(BOOST_MSVC, < 1910)
# pragma warning (pop)
#endif

/** A mutable buffers pair
*/
using mutable_buffers_pair =
    buffers_pair<false>;

/** A const buffers pair
*/
using const_buffers_pair =
    buffers_pair<true>;

//------------------------------------------------

// VFALCO GARBAGE
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
