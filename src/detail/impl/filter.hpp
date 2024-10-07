//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_FILTER_HPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_FILTER_HPP

#include <boost/buffers/algorithm.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<
    class MutableBufferSequence,
    class ConstBufferSequence>
auto
filter::
process_impl(
    MutableBufferSequence const& out,
    ConstBufferSequence const& in,
    bool more) ->
        results
{
    results rv;
    auto it_o = buffers::begin(out);
    auto it_i = buffers::begin(in);

    if(it_o == buffers::end(out) || it_i == buffers::end(in))
        return rv;

    auto ob = *it_o;
    auto ib = *it_i;
    for(;;)
    {
        if(ob.size() == 0)
        {
            if(++it_o == buffers::end(out))
                return rv;
            ob = *it_o;
        }

        if(ib.size() == 0)
        {
            if(++it_i == buffers::end(in))
                return rv;
            ib = *it_i;
        }

        results rs = process_impl(ob, ib, more);

        rv.out_bytes += rs.out_bytes;
        rv.in_bytes  += rs.in_bytes;
        rv.ec         = rs.ec;
        rv.finished   = rs.finished;

        if(rv.finished || rv.ec)
            return rv;

        ob = buffers::sans_prefix(ob, rs.out_bytes);
        ib = buffers::sans_prefix(ib, rs.in_bytes);
    }
}

} // detail
} // http_proto
} // boost

#endif
