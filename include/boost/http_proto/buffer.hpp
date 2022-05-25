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
#include <type_traits>

namespace boost {

namespace asio {
class const_buffer;
class mutable_buffer;
} // asio

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

    template<
        class Buffer,
        class = typename std::enable_if<
            std::is_same<Buffer,
                ::boost::asio::mutable_buffer
            >::value>
    >
    operator Buffer() const noexcept
    {
        return Buffer(data_, size_);
    }
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

    template<
        class Buffer,
        class = typename std::enable_if<
            std::is_same<Buffer,
                ::boost::asio::const_buffer
            >::value>
    >
    operator Buffer() const noexcept
    {
        return Buffer(data_, size_);
    }
};

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
        mutable_buffer const* data,
        std::size_t size) noexcept
        : begin_(data)
        , end_(data + size)
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
        const_buffer const* data,
        std::size_t size) noexcept
        : begin_(data)
        , end_(data + size)
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

} // http_proto
} // boost

#endif
