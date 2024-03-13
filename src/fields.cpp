//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/fields_base.hpp>       // for fields_base
#include <boost/http_proto/fields_view.hpp>       // for fields_view
#include <boost/http_proto/fields_view_base.hpp>  // for fields_view_base
#include <boost/core/detail/string_view.hpp>      // for string_view
#include <utility>                                // for move

namespace boost {
namespace http_proto {

fields::
fields() noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(
        detail::kind::fields)
{
}

fields::
fields(
    core::string_view s)
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(
        detail::kind::fields, s)
{
}

fields::
fields(
    std::size_t initial_size)
    : fields(
        initial_size, initial_size)
{
}

fields::
fields(
    std::size_t initial_size,
    std::size_t max_capacity)
    : fields_view_base(&this->fields_base::h_)
    , fields_base(
        detail::kind::fields, initial_size, max_capacity)
{
}

fields::
fields(
    fields&& other) noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(other.h_.kind)
{
    swap(other);
}

fields::
fields(
    fields const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(*other.ph_)
{
}

fields::
fields(
    fields_view const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(*other.ph_)
{
}

fields&
fields::
operator=(
    fields&& other) noexcept
{
    fields tmp(std::move(other));
    tmp.swap(*this);
    return *this;
}

} // http_proto
} // boost
