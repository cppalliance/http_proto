//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SINK_IPP
#define BOOST_HTTP_PROTO_IMPL_SINK_IPP

#include <boost/http_proto/sink.hpp>

namespace boost {
namespace http_proto {

auto
sink::
on_write(
    buffers::const_buffer_span bs,
    bool more) ->
        results
{
    auto it = bs.begin();
    auto const end_ = bs.end();
    results rv;
    if(it == end_)
        return rv;
    do
    {
        buffers::const_buffer b(*it++);
        rv += on_write(b,
            it != end_ ||
            more);
        if(rv.ec.failed())
            return rv;
    }
    while(it != end_);
    return rv;
}

} // http_proto
} // boost

#endif
