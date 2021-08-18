//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_IPP

#include <boost/http_proto/request.hpp>
#include <boost/http_proto/detail/number_string.hpp>
#include <boost/http_proto/detail/sv.hpp>

namespace boost {
namespace http_proto {

response::
response() = default;

response::
response(
    status result_code,
    http_proto::version http_version,
    string_view reason)
{
    status_line(
        result_code,
        http_version,
        reason);
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
    return "";
}

//------------------------------------------------

void
response::
status_line(
    http_proto::status result_code,
    http_proto::version http_version,
    string_view reason)
{
    if(reason.empty())
        reason = http_proto::obsolete_reason(
            result_code);

    // "HTTP/1.1 200 OK\r\n"
    auto dest = resize_start_line(
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
    detail::number_string ns;
    std::memcpy(
        dest + s.size() + 1,
        ns.data(),
        ns.size());

    //std::memcpy(
}

string_view
response::
empty_string() const noexcept
{
    return
        "HTTP/1.1 200 OK\r\n"
        "\r\n";
}

} // http_proto
} // boost

#endif
