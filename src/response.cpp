//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/version.hpp>

#include <utility>

#include "detail/header_impl.hpp"

namespace boost {
namespace http_proto {

response::
response() noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::response)
{
}

response::
response(
    core::string_view s)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::response, s)
{
}

response::
response(
    std::size_t storage_size)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::response, storage_size)
{
}

response::
response(
    std::size_t storage_size,
    std::size_t max_storage_size)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::response,
        storage_size, max_storage_size)
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
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(*other.ph_)
{
}

response::
response(
    response_view const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(*other.ph_)
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

response::
response(
    http_proto::status sc)
    : response(
        sc, http_proto::version::http_1_1)
{
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
set_impl(
    http_proto::status sc,
    unsigned short si,
    core::string_view rs,
    http_proto::version v)
{
    // measure and resize
    auto const vs = to_string(v);
    auto const n =
        vs.size() + 1 +
        3 + 1 +
        rs.size() +
        2;

    detail::prefix_op op(*this, n);
    auto dest = op.prefix_.data();

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

    h_.on_start_line();
}

} // http_proto
} // boost
