//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_SERIALIZER_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_SERIALIZER_IPP

#include <boost/http_proto/request_serializer.hpp>

namespace boost {
namespace http_proto {

void
request_serializer::
set_header(
    request_view const& res)
{
    set_header_impl(res.h_);
}

void
request_serializer::
set_header(
    request const& res)
{
    set_header_impl(&res.h_);
}

} // http_proto
} // boost

#endif
