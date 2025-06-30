//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

#include "src/detail/zlib_filter.hpp"

#include <boost/buffers/front.hpp>
#include <boost/buffers/sans_prefix.hpp>
#include <boost/buffers/size.hpp>

namespace boost {
namespace http_proto {
namespace detail {

namespace {

void*
zalloc(
    void* opaque,
    unsigned items,
    unsigned size) noexcept
{
    return reinterpret_cast<workspace*>(opaque)
        ->try_reserve_front(items * size);
}

void
zfree(
    void* /* opaque */,
    void* /* addr */) noexcept
{
    // no-op
}

unsigned int
saturate_cast(std::size_t n) noexcept
{
    if(n >= std::numeric_limits<unsigned int>::max())
        return std::numeric_limits<unsigned int>::max();
    return static_cast<unsigned int>(n);
}

} // namespace

zlib_filter::
zlib_filter(workspace& ws)
{
    strm_.zalloc = &zalloc;
    strm_.zfree  = &zfree;
    strm_.opaque = &ws;
}

auto
zlib_filter::
process(
    buffers::mutable_buffer_subspan out,
    buffers::const_buffer_pair in,
    bool more,
    bool force_flush) -> results
{
    results rv;
    auto flush = rts::zlib::no_flush;
    for(;;)
    {
        auto ob = buffers::front(out);
        auto ib = buffers::front(in);

        if(!more && flush != rts::zlib::finish && in[1].size() == 0)
        {
            if(buffers::size(out) < min_out_buffer())
            {
                rv.out_short = true;
                return rv;
            }
            flush = rts::zlib::finish;
        }

        strm_.next_in   = static_cast<unsigned char*>(const_cast<void *>(ib.data()));
        strm_.avail_in  = saturate_cast(ib.size());
        strm_.next_out  = static_cast<unsigned char*>(ob.data());
        strm_.avail_out = saturate_cast(ob.size());

        auto ec = BOOST_HTTP_PROTO_ERR(
            do_process(flush));

        const std::size_t in_bytes  = saturate_cast(ib.size()) - strm_.avail_in;
        const std::size_t out_bytes = saturate_cast(ob.size()) - strm_.avail_out;

        rv.in_bytes  += in_bytes;
        rv.out_bytes += out_bytes;

        if(ec.failed())
            return rv;

        if(ec == rts::zlib::error::stream_end)
        {
            rv.finished = true;
            return rv;
        }

        out = buffers::sans_prefix(out, out_bytes);
        in  = buffers::sans_prefix(in, in_bytes);

        if(buffers::size(out) == 0)
            return rv;

        if(buffers::size(in) == 0 && strm_.avail_out != 0)
        {
            if(force_flush && rv.out_bytes == 0)
            {
                flush = rts::zlib::block;
                continue;
            }
            return rv;
        }
    }
}

} // detail
} // http_proto
} // boost
