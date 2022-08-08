//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FILE_SOURCE_IPP
#define BOOST_HTTP_PROTO_IMPL_FILE_SOURCE_IPP

#include <boost/http_proto/file_source.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

file_source::
~file_source()
{
    delete[] buf_;
}

file_source::
file_source(
    file&& f,
    std::uint64_t offset,
    std::uint64_t size) noexcept
    : f_(std::move(f))
    , buf_(new char[4096])
    //, pos_(offset)
    , remain_(size)
    , n_(0)
    , more_(size > 0)
{
}

bool
file_source::
more() const noexcept
{
    return more_;
}

const_buffers
file_source::
prepare(error_code& ec)
{
    std::size_t n;
    if(remain_ > 0)
    {
        n = f_.read(
            buf_ + n_,
            (std::min)(
                remain_,
                4096 - n_),
            ec);
        n_ += n;
        remain_ -= n;
    }
    else
    {
        n = 0;
    }
    asio::const_buffer b( buf_, n_ );
    return const_buffers(&b, 1);
}

void
file_source::
consume(std::size_t n) noexcept
{
    BOOST_ASSERT(n <= n_);
    if(n < n_)
        std::memmove(
            buf_,
            buf_ + n,
            n_ - n);
    n_ -= n;
}

} // http_proto
} // boost

#endif
