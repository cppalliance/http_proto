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
#include <boost/http_proto/detail/number_string.hpp>
#include <boost/http_proto/detail/sv.hpp>
#include <utility>

namespace boost {
namespace http_proto {

response::
response()
    : version_(http_proto::version::http_1_1)
    , result_(http_proto::status::ok)
    , fields(2)
{
}

response::
response(response&& other) noexcept
    : response()
{
    swap(other);
}

response::
response(response const& other)
    : basic_header()
    , version_(other.version_)
    , result_(other.result_)
    , fields(other.fields, 2)
{
}

response&
response::
operator=(response&& other) noexcept
{
    response temp(
        std::move(other));
    swap(temp);
    return *this;
}

response&
response::
operator=(response const& other)
{
    response temp(other);
    swap(temp);
    return *this;
}

//------------------------------------------------

status
response::
result() const noexcept
{
    return int_to_status(
        static_cast<int>(result_));
}

unsigned
response::
result_int() const noexcept
{
    return static_cast<
        unsigned>(result_);
}

string_view
response::
reason() const noexcept
{
    return {};
}

response::
operator response_view() const noexcept
{
    return {};
}

string_view
response::
get_const_buffer() const noexcept 
{
    return fields.owner_str();
}

//------------------------------------------------

void
response::
clear() noexcept
{
    if(! fields.buf_)
        return;

}

void
response::
set_result(
    status code,
    http_proto::version http_version,
    string_view reason)
{
    if(reason.empty())
        reason = obsolete_reason(code);

    // "HTTP/1.1 200 OK\r\n"
    auto dest = fields.set_start_line(
        8 + 1 + 3 + 1 + reason.size() + 2);

    string_view s;

    // version
    s = to_string(http_version);
    std::memcpy(
        dest, s.data(), s.size());
    dest += s.size();

    // SP
    *dest++ = ' ';

    // status-code
    auto const i =
        static_cast<unsigned>(
            code);
    *dest++ = '0' + ((i / 100) % 10);
    *dest++ = '0' + ((i /  10) % 10);
    *dest++ = '0' + ((i /   1) % 10);

    // SP
    *dest++ = ' ';

    // obsolete-reason
    dest += reason.copy(
        dest, reason.size());

    // CRLF
    dest[0] = '\r';
    dest[1] = '\n';

    version_ = http_version;
    result_ = code;
}

void
response::
swap(response& other) noexcept
{
    std::swap(version_, other.version_);
    std::swap(result_, other.result_);
    fields.owner_swap(other.fields);
}

} // http_proto
} // boost

#endif
