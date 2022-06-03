//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_SERIALIZER_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_SERIALIZER_IPP

#include <boost/http_proto/response_serializer.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_view.hpp>

namespace boost {
namespace http_proto {

void
response_serializer::
set_header(
    response_view const& res)
{
    set_header_impl(res.h_);
}

void
response_serializer::
set_header(
    response const& res)
{
    set_header_impl(&res.h_);
}

} // http_proto
} // boost

#endif
