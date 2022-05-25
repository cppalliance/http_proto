//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_IPP

#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/detail/copied_strings.hpp>
#include <utility>

namespace boost {
namespace http_proto {

request::
request()
    : fields_base(1)
    , method_(http_proto::method::get)
    , version_(http_proto::version::http_1_1)
    , method_len_(3)
    , target_len_(1)
{
}

request::
request(
    request const& other)
    : fields_base(other, 1)
    , method_(other.method_)
    , version_(other.version_)
    , method_len_(other.method_len_)
    , target_len_(other.target_len_)
{
    BOOST_ASSERT(kind_ == 1);
}

request::
request(request&& other) noexcept
    : request()
{
    swap(other);
}

request&
request::
operator=(request&& other) noexcept
{
    request temp(
        std::move(other));
    temp.swap(*this);
    return *this;
}

request&
request::
operator=(
    request const& other)
{
    request temp(other);
    swap(temp);
    return *this;
}

request::
request(
    request_view const& rv)
    : fields_base(rv, 1)
    , method_(rv.method_)
    , version_(rv.version_)
    , method_len_(rv.method_len_)
    , target_len_(rv.target_len_)
{
}

//------------------------------------------------

request::
operator
request_view() const noexcept
{
    request_view::ctor_params init;
    init.cbuf = cbuf_;
    init.buf_len = buf_len_;
    init.start_len = start_len_;
    init.end_pos = end_pos_;
    init.count = count_;
    init.method_len = method_len_;
    init.target_len = target_len_;
    init.method = method_;
    init.version = version_;
    return request_view(init);
}

request::
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
request::
clear() noexcept
{
    if(buf_ == nullptr)
        return;
    this->fields_base::clear();
    set_impl(
        http_proto::method::get,
        "GET",
        "/",
        http_proto::version::http_1_1);
}

void
request::
swap(request& other) noexcept
{
    this->fields_base::swap(other);
    std::swap(method_, other.method_);
    std::swap(version_, other.version_);
    std::swap(method_len_, other.method_len_);
    std::swap(target_len_, other.target_len_);
}

//------------------------------------------------

void
request::
set_impl(
    http_proto::method m,
    string_view ms,
    string_view t,
    http_proto::version v)
{
    detail::copied_strings cs(
        this->string());
    ms = cs.maybe_copy(ms);
    t = cs.maybe_copy(t);

    auto const vs =
        to_string(v);
    auto const n =
        ms.size() + 1 +
        t.size() + 1 +
        vs.size() + 2;
    auto dest =
        set_start_line_impl(n);
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
