//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_CIRCULAR_BUFFER_HPP
#define BOOST_HTTP_PROTO_CIRCULAR_BUFFER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/detail/except.hpp>

namespace boost {
namespace http_proto {

class circular_buffer
{
    char* begin_;
    std::size_t in_off_ = 0;
    std::size_t in_size_ = 0;
    std::size_t out_size_ = 0;
    std::size_t capacity_ = 0;
    const_buffer cb_[2];
    mutable_buffer mb_[2];

public:
    using const_buffers_type = const_buffers;
    using mutable_buffers_type = mutable_buffers;

    circular_buffer(
        void* data,
        std::size_t size) noexcept
        : begin_(reinterpret_cast<
            char*>(data))
        , capacity_(size)
    {
    }

    void
    clear() noexcept
    {
        in_off_ = 0;
        in_size_ = 0;
        out_size_ = 0;
    }

    /// Returns the number of readable bytes.
    std::size_t
    size() const noexcept
    {
        return in_size_;
    }

    /// Return the maximum number of bytes, both readable and writable, that can ever be held.
    std::size_t
    max_size() const noexcept
    {
        return capacity_;
    }

    /// Return the maximum number of bytes, both readable and writable, that can be held without requiring an allocation.
    std::size_t
    capacity() const noexcept
    {
        return capacity_;
    }

    const_buffers_type
    data() const noexcept
    {
        if(cb_[1].size() > 0)
            return { &cb_[0], 2 };
        return { &cb_[0], 1 };
    }

    mutable_buffers_type
    prepare(std::size_t n)
    {
        if(n > capacity_ - in_size_)
            detail::throw_length_error(
                "overflow");
        out_size_ = n;
        auto const out_off =
            (in_off_ + in_size_) % capacity_;
        if(out_off + out_size_ <= capacity_)
        {
            mb_[0] = {
                begin_ + out_off,
                out_size_ };
            mb_[1] = {};
            return {&mb_[0], 1};
        }
        mb_[0] = {
            begin_ + out_off,
            capacity_ - out_off};
        mb_[1] = {
            begin_,
            out_size_ - (
                capacity_ - out_off)};
        return {&mb_[0], 2};
    }

    void
    commit(std::size_t n) noexcept
    {
        if(n > out_size_)
            n = out_size_;
        in_size_ += n;
        out_size_ = 0;
        update();
    }

    void
    consume(std::size_t n) noexcept
    {
        if(n < in_size_)
        {
            in_off_ = (in_off_ + n) % capacity_;
            in_size_ -= n;
        }
        else
        {
            // rewind the offset, so the next call to prepare
            // can have a longer contiguous segment. this helps
            // algorithms optimized for larger buffers.
            in_off_ = 0;
            in_size_ = 0;
        }
        update();
    }

private:
    void
    update()
    {
        if(capacity_ >=
            in_off_ + in_size_)
        {
            cb_[0] = {
                begin_ + in_off_,
                in_size_};
            cb_[1] = {};
            return;
        }
        cb_[0] = {
            begin_ + in_off_,
            capacity_ - in_off_};
        cb_[1] = {
            begin_,
            in_size_ - (
                capacity_ - in_off_)};
    }
};

} // http_proto
} // boost

#endif
