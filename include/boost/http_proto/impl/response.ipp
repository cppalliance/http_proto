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

    version_ = v;
    vs.copy(dest, vs.size());
    dest += vs.size();
    *dest++ = ' ';

    status_ = sc;
    status_int_ = si;
    dest[0] = '0' + ((status_int_ / 100) % 10);
    dest[1] = '0' + ((status_int_ /  10) % 10);
    dest[2] = '0' + ((status_int_ /   1) % 10);
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
    if( sc != status_ ||
        v != version_)
        set_start_line(sc, v);
}

//------------------------------------------------

response::
response() noexcept
    : fields_base(2)
    , version_(http_proto::version::http_1_1)
    , status_(http_proto::status::ok)
    , status_int_(200)
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
    : fields_base(other, 2)
    , version_(other.version_)
    , status_(other.status_)
    , status_int_(other.status_int_)
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
    : fields_base(rv, 2)
{
    status_ = rv.status();
    status_int_ = rv.status_int();
    version_ = rv.version();
}

response&
response::
operator=(
    response_view const& rv)
{
    this->fields_base::copy(rv);
    status_ = rv.status();
    status_int_ = rv.status_int();
    version_ = rv.version();
    return *this;
}

//------------------------------------------------

response::
operator
response_view() const noexcept
{
    response_view::ctor_params init;
    init.cbuf = cbuf_;
    init.buf_len = buf_len_;
    init.start_len = start_len_;
    init.end_pos = end_pos_;
    init.count = count_;
    init.version = version_;
    init.status = status_;
    init.status_int = status_int_;
    return response_view(init);
}

response::
operator
header_info() const noexcept
{
    return {
        cbuf_,
        buf_len_,
        nullptr
    };
}

//------------------------------------------------

void
response::
clear() noexcept
{
    if(buf_ == nullptr)
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
    std::swap(version_, other.version_);
    std::swap(status_, other.status_);
    std::swap(status_int_, other.status_int_);
}

//------------------------------------------------

} // http_proto
} // boost

#endif
