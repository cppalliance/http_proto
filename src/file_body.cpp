//
// Copyright (c) 2022 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/file_body.hpp>
#include <boost/buffers/algorithm.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

file_body::
~file_body() = default;

file_body::
file_body(
    file_body&&) noexcept = default;

file_body::
file_body(
    file&& f,
    std::uint64_t size) noexcept
    : f_(std::move(f))
    , n_(size)
{
}

auto
file_body::
on_read(
    buffers::mutable_buffer b) ->
        results
{
    results rv;
    if(n_ > 0)
    {
        std::size_t n;
        if( n_ >= b.size())
            n = b.size();
        else
            n = n_;
        n = f_.read(
            b.data(), n, rv.ec);
        rv.bytes = n;
        n_ -= n;
    }
    rv.finished = n_ == 0;
    return rv;
}

} // http_proto
} // boost
