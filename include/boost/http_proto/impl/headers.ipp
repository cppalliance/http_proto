//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_HTTP_PROTO_IMPL_HEADERS_IPP
#define BOOST_HTTP_PROTO_IMPL_HEADERS_IPP

#include <boost/http_proto/headers.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/ftab.hpp>
#include <utility>

namespace boost {
namespace http_proto {

std::size_t
headers::
align_up(std::size_t n) noexcept
{
    auto const a = sizeof(
        detail::fitem);
    return a * ((n + a - 1) / a);
}

headers::
~headers()
{
    if(buf_)
        delete[] buf_;
}

headers::
headers() noexcept = default;

char*
headers::
resize_prefix(
    std::size_t n)
{
    if(! buf_)
    {
        buf_ = new char[1024];
        count_ = 0;
        capacity_ = 1024;
        prefix_bytes_ = n;
        fields_bytes_ = 0;

        buf_[n] = '\r';
        buf_[n+1] = '\n';
    }

    return buf_;
}

//------------------------------------------------

void
headers::
append(
    field f,
    string_view value)
{
    append(f, to_string(f), value);
}

void
headers::
append(
    string_view name,
    string_view value)
{
    append(string_to_field(name),
        name, value);
}

//------------------------------------------------

void
headers::
append(
    field id,
    string_view name,
    string_view value)
{
    auto const n =
        name.size() + 2 +
        value.size() + 2;

    // calculate new size
    auto const need = align_up(
        prefix_bytes_ +
        fields_bytes_ + n + 2 +
        sizeof(detail::fitem) *
            (count_ + 1));
    // VFALCO we shouldn't count the
    // fields table towards the max
    if(need > off_t(-1))
        detail::throw_length_error(
            "too long",
            BOOST_CURRENT_LOCATION);

    if(buf_ != nullptr)
    {
        if(capacity_ < need)
        {
            char* buf = new char[need];
            std::memcpy(buf, buf_,
                prefix_bytes_ +
                fields_bytes_);
            auto const tabsize = sizeof(
                detail::fitem) * count_;
            std::memcpy(
                buf + need - tabsize,
                buf_ + capacity_ - tabsize,
                tabsize);
            std::swap(buf_, buf);
            delete[] buf;
            capacity_ = need;
        }
    }
    else
    {
        buf_ = new char[need];
        count_ = 0;
        capacity_ = need;
        fields_bytes_ = 0;
        prefix_bytes_ = 0;
    }

    auto dest = buf_ +
        prefix_bytes_ +
        fields_bytes_;
    auto& ft = detail::get_ftab(
        buf_ + capacity_)[count_];
    ft.id = id;
    ft.name_len = static_cast<
        off_t>(name.size());
    ft.value_len = static_cast<
        off_t>(value.size());
    ft.name_pos = static_cast<
        off_t>(dest - buf_);
    std::memcpy(
        dest,
        name.data(),
        name.size());
    dest += name.size();
    *dest++ = ':';
    *dest++ = ' ';
    ft.value_pos = static_cast<
        off_t>(dest - buf_);
    std::memcpy(
        dest,
        value.data(),
        value.size());
    dest += value.size();
    *dest++ = '\r';
    *dest++ = '\n';
    *dest++ = '\r';
    *dest++ = '\n';
    fields_bytes_ =
        dest - buf_ -
            prefix_bytes_;
    ++count_;
}

} // http_proto
} // boost

#endif
