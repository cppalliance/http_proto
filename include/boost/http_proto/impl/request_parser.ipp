//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP

#include <boost/http_proto/request_parser.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

request_parser::
request_parser()
    : request_parser(65536)
{
}

request_parser::
request_parser(
    std::size_t buffer_bytes)
    : parser(
        detail::kind::request,
        buffer_bytes)
{
}

request_view
request_parser::
get() const noexcept
{
    return request_view(&h_);
}

} // http_proto
} // boost

#endif
