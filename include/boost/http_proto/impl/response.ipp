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

void
response::
set_start_line_impl(
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
    auto dest = this->fields_base::
        set_start_line_impl(n);

    h_.res.version = v;
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

response::
response(
    http_proto::status sc,
    http_proto::version v)
    : response()
{
    if( sc != h_.res.status ||
        v != h_.res.version)
        set_start_line(sc, v);
}

//------------------------------------------------

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
    : fields_base(other,
        detail::kind::response)
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
    response temp(other);
    this->swap(temp);
    return *this;
}

//------------------------------------------------

response::
response(
    response_view const& rv)
    : fields_base(rv,
        detail::kind::response)
{
}

response&
response::
operator=(
    response_view const& rv)
{
    this->fields_base::copy(rv);
    h_.res.status = rv.status();
    h_.res.status_int = rv.status_int();
    h_.res.version = rv.version();
    return *this;
}

//------------------------------------------------

response::
operator
response_view() const noexcept
{
    return response_view(h_);
}

//------------------------------------------------

void
response::
clear() noexcept
{
    if(h_.buf == nullptr)
        return;
    this->fields_base::clear();
    set_start_line(
        http_proto::status::ok);
}

void
response::
swap(response& other) noexcept
{
    this->fields_base::swap(other);
}

//------------------------------------------------

} // http_proto
} // boost

#endif
