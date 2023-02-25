//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_BUFFERS_IMPL_FILTER_IPP
#define BOOST_BUFFERS_IMPL_FILTER_IPP

#include <boost/http_proto/filter.hpp>
#include <boost/buffers/range.hpp>

namespace boost {
namespace http_proto {

auto
filter::
on_process(
    buffers::mutable_buffer_span out,
    buffers::const_buffer_span in,
    bool more) ->
        results
{
    results rv;
    auto it0 = buffers::begin(in);
    auto it1 = buffers::begin(out);
    auto const end0 = buffers::end(in);
    auto const end1 = buffers::end(out);
    while(
        it0 != end0 ||
        it1 != end1)
    {
        ++it1;
        (void)more;
        (void)end0;
        (void)it0;
    }
    return rv;
}

} // http_proto
} // boost

#endif
