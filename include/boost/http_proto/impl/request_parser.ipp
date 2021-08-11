//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP

#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/bnf/request_line.hpp>
#include <boost/http_proto/bnf/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

request_parser::
request_parser(
    context& ctx) noexcept
    : basic_parser(ctx)
    , method_(http_proto::method::unknown)
    , n_method_(0)
    , n_target_(0)
{
}

request_view
request_parser::
header() const noexcept
{
    return request_view(
        buffer_,
        used_,
        n_method_,
        n_target_,
        method_,
        version_);
}

//------------------------------------------------

char*
request_parser::
parse_start_line(
    char* const start,
    char const* const end,
    error_code& ec)
{
    bnf::request_line p;
    auto it = p.parse_element(
        start, end, ec);
    if(ec)
        return start + (it - start);
    method_ = string_to_method(
        p.value.method);
    n_method_ = static_cast<
        off_t>(p.value.method.size());
    n_target_ = static_cast<
        off_t>(p.value.target.size());
    version_ = static_cast<char>(
        p.value.version);
    return start + (it - start);
}

void
request_parser::
finish_header(
    error_code& ec)
{
    (void)ec;
}

} // http_proto
} // boost

#endif
