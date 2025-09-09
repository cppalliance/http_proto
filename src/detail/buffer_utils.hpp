//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_BUFFER_UTILS_HPP
#define BOOST_HTTP_PROTO_DETAIL_BUFFER_UTILS_HPP

#include <boost/buffers/buffer.hpp>
#include <boost/buffers/slice.hpp>
#include <boost/core/span.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<
    typename BufferSequence,
    typename Buffer = typename BufferSequence::value_type>
boost::span<Buffer const>
make_span(BufferSequence const& mbp)
{
    return { mbp.begin(), mbp.end() };
}

template<typename BufferSequence>
BufferSequence
prefix(
    BufferSequence cbp,
    std::size_t n)
{
    buffers::keep_front(cbp, n);
    return cbp;
}

} // detail
} // http_proto
} // boost

#endif
