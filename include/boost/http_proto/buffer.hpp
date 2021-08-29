//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BUFFER_HPP
#define BOOST_HTTP_PROTO_BUFFER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <cstdlib>

namespace boost {

#ifndef BOOST_HTTP_PROTO_DOCS
namespace asio {
class const_buffer;
class mutable_buffer;
} // asio
#endif

namespace http_proto {

/** Holds a buffer that can be modified
*/
class mutable_buffer
{
    void* data_ = nullptr;
    std::size_t size_ = 0;

public:
    mutable_buffer() noexcept = default;

    mutable_buffer(
        void* data,
        std::size_t size) noexcept
        : data_(data)
        , size_(size)
    {
    }

    void*
    data() const noexcept
    {
        return data_;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    mutable_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n > size_)
            n = size_;
        data_ = static_cast<
            char*>(data_) + n;
        size_ -= n;
        return *this;
    }

#ifndef BOOST_HTTP_PROTO_DOCS
    inline
    operator
    asio::mutable_buffer() const noexcept;
#endif
};

//------------------------------------------------

/** Holds a buffer that cannot be modified
*/
class const_buffer
{
    void const* data_ = nullptr;
    std::size_t size_ = 0;

public:
    const_buffer() noexcept = default;

    const_buffer(
        mutable_buffer const& mb) noexcept
        : data_(mb.data())
        , size_(mb.size())
    {
    }

    const_buffer(
        void const* data,
        std::size_t size) noexcept
        : data_(data)
        , size_(size)
    {
    }

    void const*
    data() const noexcept
    {
        return data_;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    const_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n > size_)
            n = size_;
        data_ = static_cast<
            char const*>(data_) + n;
        size_ -= n;
        return *this;
    }

#ifndef BOOST_HTTP_PROTO_DOCS
    inline
    operator
    asio::const_buffer() const noexcept;
#endif
};

//------------------------------------------------

inline
mutable_buffer
operator+(
    mutable_buffer b,
    std::size_t n) noexcept
{
    b += n;
    return b;
}

inline
mutable_buffer
operator+(
    std::size_t n,
    mutable_buffer b) noexcept
{
    b += n;
    return b;
}

inline
const_buffer
operator+(
    const_buffer b,
    std::size_t n) noexcept
{
    b += n;
    return b;
}

inline
const_buffer
operator+(
    std::size_t n,
    const_buffer b) noexcept
{
    b += n;
    return b;
}

//------------------------------------------------

/** Holds a buffer sequence that cannot be modified
*/
class mutable_buffers
{
    mutable_buffer const* begin_ = nullptr;
    mutable_buffer const* end_ = nullptr;

public:
    mutable_buffers() = default;
    mutable_buffers(
        mutable_buffers const&) = default;
    mutable_buffers&
    operator=(mutable_buffers const&) = default;

    mutable_buffers(
        mutable_buffer const* begin,
        mutable_buffer const* end) noexcept
        : begin_(begin)
        , end_(end)
    {
    }

    mutable_buffer const*
    begin() const noexcept
    {
        return begin_;
    }

    mutable_buffer const*
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
    const_buffer const* begin_ = nullptr;
    const_buffer const* end_ = nullptr;

public:
    const_buffers() = default;
    const_buffers(
        const_buffers const&) = default;
    const_buffers&
    operator=(const_buffers const&) = default;

    const_buffers(
        const_buffer const* begin,
        const_buffer const* end) noexcept
        : begin_(begin)
        , end_(end)
    {
    }

    const_buffer const*
    begin() const noexcept
    {
        return begin_;
    }

    const_buffer const*
    end() const noexcept
    {
        return end_;
    }
};

struct const_buffer_pair
{
    char const* data[2];
    std::size_t size[2];
};

} // http_proto
} // boost

#endif
