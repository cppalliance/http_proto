//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/response_base.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/fields_base.hpp>

#include <cstring>
#include <ostream>

namespace boost {
namespace http_proto {

void
response_base::
set_start_line_impl(
    http_proto::status sc,
    unsigned short si,
    core::string_view rs,
    http_proto::version v)
{
    // TODO: check validity
    auto const vs = to_string(v);
    auto const new_prefix =
        vs.size() + 1 + // HTTP-version SP
        3 + 1 +         // status-code SP
        rs.size() + 2;  // reason-phrase CRLF

    // Introduce a new scope so that prefix_op's
    // destructor runs before h_.on_start_line().
    {
        auto op = prefix_op_t(*this, new_prefix, &rs);
        char* dest = h_.buf;

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

        std::memmove(dest, rs.data(), rs.size());
        dest += rs.size();
        dest[0] = '\r';
        dest[1] = '\n';
    }

    h_.on_start_line();
}

//------------------------------------------------

std::ostream&
operator<<(
    std::ostream& os,
    const response_base& res)
{
    return operator<<(os, static_cast<const fields_base&>(res));
}

//------------------------------------------------

std::ostream&
operator<<(
    std::ostream& os,
    const response& res)
{
    return operator<<(os, static_cast<const response_base&>(res));
}

} // http_proto
} // boost
