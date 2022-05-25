//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_HEADER_INFO_HPP
#define BOOST_HTTP_PROTO_HEADER_INFO_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

struct header_meta
{
    std::uint64_t content_length;

    bool has_chunked : 1;
    bool has_content_length : 1;
};

/** A low-level description of a serializable header
*/
struct header_info
{
    char const* data;
    std::size_t size;
    header_meta const* meta;
};

} // http_proto
} // boost

#endif
