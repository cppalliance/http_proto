//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

#include <boost/http_proto/source.hpp>
#include <boost/buffers/mutable_buffer.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

auto
source::
on_read(
    buffers::mutable_buffer_span bs) ->
        results
{
    results rv;
    auto it = bs.begin();
    auto const end_ = bs.end();
    if(it == end_)
        return rv;
    do
    {
        buffers::mutable_buffer b(*it++);
        rv += on_read(b);
        if(rv.ec.failed())
            return rv;
        if(rv.finished)
            break;
    }
    while(it != end_);
    return rv;
}

} // http_proto
} // boost
