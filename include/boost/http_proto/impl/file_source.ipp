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
~file_source() = default;

file_source::
file_source(
    file_source&&) noexcept = default;

file_source::
file_source(
    file&& f,
    std::uint64_t /*offset*/,
    std::uint64_t size) noexcept
    : f_(std::move(f))
    //, pos_(offset)
    , n_(size)
{
}

bool
file_source::
more() const noexcept
{
    return n_ > 0;
}

std::size_t
file_source::
write(
    void* dest,
    std::size_t size,
    error_code& ec)
{
    std::size_t n;
    if(n_ > 0)
    {
        if( n_ >= size)
            n = size;
        else
            n = n_;
        n = f_.read(
            dest, n, ec);
        n_ -= n;
    }
    else
    {
        n = 0;
    }
    return n;
}

} // http_proto
} // boost

#endif
