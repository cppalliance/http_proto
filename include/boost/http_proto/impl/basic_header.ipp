//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_BASIC_HEADER_IPP
#define BOOST_HTTP_PROTO_IMPL_BASIC_HEADER_IPP

#include <boost/http_proto/basic_header.hpp>

namespace boost {
namespace http_proto {

basic_header::
basic_header() noexcept
    : content_length_(0)
    , has_chunked_(false)
    , has_content_length_(false)
{
}

basic_header::
basic_header(
    ctor_params const& init) noexcept
    : content_length_(init.content_length)
    , has_chunked_(init.has_chunked)
    , has_content_length_(init.has_content_length)
{
}

basic_header::
~basic_header() = default;

void
basic_header::
swap(basic_header& other) noexcept
{
    (void)other;
}

} // http_proto
} // boost

#endif
