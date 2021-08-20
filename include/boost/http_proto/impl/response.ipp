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

//------------------------------------------------

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
    auto dest = fields.resize_prefix(
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
            result_);
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

} // http_proto
} // boost

#endif
