//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_VIEW_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_VIEW_IPP

#include <boost/http_proto/request_view.hpp>

namespace boost {
namespace http_proto {

request_view::
request_view(
    char const* buf,
    std::size_t count,
    std::size_t start_bytes,
    std::size_t fields_bytes,
    std::size_t capacity,
    std::size_t method_len,
    std::size_t target_len,
    http_proto::method method,
    http_proto::version version) noexcept
    : method_len_(method_len)
    , target_len_(target_len)
    , method_(method)
    , version_(version)
    , fields(
        buf,
        count,
        start_bytes,
        fields_bytes,
        capacity)
{
}

request_view::
request_view() noexcept = default;

string_view
request_view::
get_const_buffer() const noexcept
{
    return fields.str_impl();
}

} // http_proto
} // boost

#endif
