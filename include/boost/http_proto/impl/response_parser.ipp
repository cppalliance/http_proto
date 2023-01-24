//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_PARSER_IPP

#include <boost/http_proto/response_parser.hpp>

namespace boost {
namespace http_proto {

response_parser::
response_parser()
    : response_parser(65536)
{
}

response_view
response_parser::
get() const
{
    return response_view(
        safe_get_header());
}

} // http_proto
} // boost

#endif
