//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_IPP

#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/detail/copied_strings.hpp>
#include <utility>

namespace boost {
namespace http_proto {

response::
response() noexcept
    : fields_base(
        detail::kind::response)
{
}

response::
response(
    response&& other) noexcept
    : response()
{
    swap(other);
}

response::
response(
    response const& other)
    : fields_base(*other.ph_)
{
}

response::
response(
    response_view const& other)
    : fields_base(*other.ph_)
{
}

response&
response::
operator=(
    response&& other) noexcept
{
    response temp(
        std::move(other));
    temp.swap(*this);
    return *this;
}

response&
response::
operator=(
    response const& other)
{
    copy_impl(*other.ph_);
    return *this;
}

response&
response::
operator=(
    response_view const& other)
{
    copy_impl(*other.ph_);
    return *this;
}

response::
response(
    http_proto::status sc,
    http_proto::version v)
    : response()
{
    if( sc != h_.res.status ||
        v != h_.version)
        set_start_line(sc, v);
}

//------------------------------------------------

void
response::
clear() noexcept
{
    if(h_.buf == nullptr)
    {
        // default buffer
        return;
    }
    clear_impl();
    set_start_line(
        http_proto::status::ok);
}

//------------------------------------------------

void
response::
set_impl(
    http_proto::status sc,
    unsigned short si,
    string_view rs,
    http_proto::version v)
{
    // measure and resize
    auto const vs = to_string(v);
    auto const n =
        vs.size() + 1 +
        3 + 1 +
        rs.size() +
        2;
    auto dest = set_prefix_impl(n);

    h_.version = v;
    vs.copy(dest, vs.size());
    dest += vs.size();
    *dest++ = ' ';

    h_.res.status = sc;
    h_.res.status_int = si;
    dest[0] = '0' + ((h_.res.status_int / 100) % 10);
    dest[1] = '0' + ((h_.res.status_int /  10) % 10);
    dest[2] = '0' + ((h_.res.status_int /   1) % 10);
    dest[3] = ' ';
    dest += 4;

    rs.copy(dest, rs.size());
    dest += rs.size();
    dest[0] = '\r';
    dest[1] = '\n';
}

} // http_proto
} // boost

#endif
