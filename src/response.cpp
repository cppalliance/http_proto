//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/response.hpp>
#include <boost/http_proto/version.hpp>

#include <utility>

namespace boost {
namespace http_proto {

response::
response() noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , response_base()
{
}

response::
response(
    core::string_view s)
    : fields_view_base(
        &this->fields_base::h_)
    , response_base(s)
{
}

response::
response(
    std::size_t cap,
    std::size_t max_cap)
    : response()
{
    reserve_bytes(cap);
    set_max_capacity_in_bytes(max_cap);
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
    , response_base(*other.ph_)
{
}

response::
response(
    response_view const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , response_base(*other.ph_)
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
    set_start_line(sc, v);
}

} // http_proto
} // boost
