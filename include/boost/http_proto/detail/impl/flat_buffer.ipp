//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_FLAT_BUFFER_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_FLAT_BUFFER_IPP

#include <boost/http_proto/detail/flat_buffer.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace detail {

flat_buffer::
flat_buffer(
    void* data,
    std::size_t capacity) noexcept
    : data_(reinterpret_cast<
        unsigned char*>(data))
    , cap_(capacity)
{
}

bool
flat_buffer::
empty() const noexcept
{
    return in_size_ == 0;
}

std::size_t
flat_buffer::
size() const noexcept
{
    return in_size_;
}

std::size_t
flat_buffer::
capacity() const noexcept
{
    return cap_;
}

const_buffer
flat_buffer::
data() const noexcept
{
    return {
        data_ + in_pos_,
        in_size_ };
}

mutable_buffer
flat_buffer::
prepare(std::size_t n)
{
    // n exceeds available space
    if(n > cap_ - in_size_)
        detail::throw_invalid_argument();

    out_size_ = n;
    return {
        data_ + in_pos_ + in_size_,
        n };
}

void
flat_buffer::
commit(std::size_t n)
{
    // n exceeds output size
    if(n > out_size_)
        detail::throw_invalid_argument();

    in_size_ += n;
    out_size_ = 0;
}

void
flat_buffer::
consume(std::size_t n) noexcept
{
    BOOST_ASSERT(out_size_ == 0);

    // n exceeds input size
    if(n > in_size_)
        detail::throw_invalid_argument();

    in_size_ -= n;
    if(in_size_ > 0)
        in_pos_ += n;
    else
        in_pos_ = 0;
}

} // detail
} // http_proto
} // boost

#endif
