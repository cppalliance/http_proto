//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_IPP

#include <boost/http_proto/request.hpp>
#include <boost/http_proto/detail/copied_strings.hpp>

namespace boost {
namespace http_proto {

request::
request()
    : method_(method::get)
    , version_(http_proto::version::http_1_1)
    , method_len_(3)
    , target_len_(1)
    , fields(
        "GET / HTTP/1.1\r\n"
        "\r\n")
{
}

request::
request(request&&) noexcept = default;

request::
request(request const&) = default;

//------------------------------------------------

string_view
request::
get_const_buffer() const noexcept
{
    return fields.str_impl();
}

//------------------------------------------------

void
request::
clear() noexcept
{
    version_ =
        http_proto::version::http_1_1;
    method_len_ = 3;
    target_len_ = 1;
    fields.clear();
}

void
request::
set(http_proto::method m,
    string_view ms,
    string_view t,
    http_proto::version v)
{
    detail::copied_strings cs(
        fields.str_impl());
    ms = cs.maybe_copy(ms);
    t = cs.maybe_copy(t);

    auto const vs =
        to_string(v);
    auto const n =
        ms.size() + 1 +
        t.size() + 1 +
        vs.size() + 2;
    auto dest =
        fields.set_start_line(n);
    std::memcpy(
        dest,
        ms.data(),
        ms.size());
    dest += ms.size();
    *dest++ = ' ';
    std::memcpy(
        dest,
        t.data(),
        t.size());
    dest += t.size();
    *dest++ = ' ';
    std::memcpy(
        dest,
        vs.data(),
        vs.size());
    dest += vs.size();
    *dest++ = '\r';
    *dest++ = '\n';

    method_ = m;
    version_ = v;
    method_len_ = ms.size();
    target_len_ = t.size();
}

} // http_proto
} // boost

#endif
