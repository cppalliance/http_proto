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
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/http_proto/rfc/status_line_rule.hpp>

namespace boost {
namespace http_proto {

response_parser::
response_parser(
    config const& cfg,
    std::size_t buffer_size)
    : parser(
        detail::kind::response,
        cfg,
        buffer_size)
{
}

response_view
response_parser::
get() const noexcept
{
    return response_view(h_);
}

} // http_proto
} // boost

#endif
