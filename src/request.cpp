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

#include <boost/http_proto/request.hpp>

namespace boost {
namespace http_proto {

request::
request() noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , request_base()
{
}

request::
request(
    core::string_view s)
    : fields_view_base(
        &this->fields_base::h_)
    , request_base(s)
{
}

request::
request(
    std::size_t storage_size)
    : fields_view_base(
        &this->fields_base::h_)
    , request_base(storage_size)
{
}

request::
request(
    std::size_t storage_size,
    std::size_t max_storage_size)
    : fields_view_base(
        &this->fields_base::h_)
    , request_base(
        storage_size,
        max_storage_size)
{
}

request::
request(
    request&& other) noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , request_base()
{
    swap(other);
}

request::
request(
    request const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , request_base(*other.ph_)
{
}

request::
request(
    request_view const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , request_base(*other.ph_)
{
}

request&
request::
operator=(
    request&& other) noexcept
{
    request temp(
        std::move(other));
    temp.swap(*this);
    return *this;
}

} // http_proto
} // boost
