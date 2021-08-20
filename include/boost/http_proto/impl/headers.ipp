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
#include <boost/http_proto/bnf/ctype.hpp>
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

//------------------------------------------------

auto
headers::
operator[](
    std::size_t i) const noexcept ->
        value_type const
{
    auto const ft =
        detail::get_ftab(
            buf_ + capacity_)[i];
    return value_type {
        ft.id,
        string_view(
            buf_ + ft.name_pos,
            ft.name_len),
        string_view(
            buf_ + ft.value_pos,
            ft.value_len) };
}

bool
headers::
exists(
    field id) const noexcept
{
    return find(id) != end();
}

bool
headers::
exists(
    string_view name) const noexcept
{
    return find(name) != end();
}

std::size_t
headers::
count(field id) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(v.id == id)
            ++n;
    return n;
}

std::size_t
headers::
count(string_view name) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(bnf::iequals(
            v.name, name))
            ++n;
    return n;
}

auto
headers::
at(std::size_t i) const ->
    value_type const
{
    if(i < count_)
    {
        auto const ft =
            detail::get_ftab(
                buf_ + capacity_)[i];
        return value_type {
            ft.id,
            string_view(
                buf_ + ft.name_pos,
                ft.name_len),
            string_view(
                buf_ + ft.value_pos,
                ft.value_len) };
    }
    detail::throw_invalid_argument(
        "bad index", BOOST_CURRENT_LOCATION);
}

string_view
headers::
at(field id) const
{
    auto it = find(id);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found", BOOST_CURRENT_LOCATION);
}

string_view
headers::
at(string_view name) const
{
    auto it = find(name);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found", BOOST_CURRENT_LOCATION);
}

string_view
headers::
value_or(
    field id,
    string_view v) const noexcept
{
    auto it = find(id);
    if(it != end())
        return it->value;
    return v;
}

string_view
headers::
value_or(
    string_view name,
    string_view v) const noexcept
{
    auto it = find(name);
    if(it != end())
        return it->value;
    return v;
}

auto
headers::
find(field id) const noexcept ->
    iterator
{
    auto it = begin();
    auto const last = end();
    while(it != last)
    {
        if(it->id == id)
            break;
        ++it;
    }
    return it;
}

auto
headers::
find(string_view name) const noexcept ->
    iterator
{
    auto it = begin();
    auto const last = end();
    while(it != last)
    {
        if(bnf::iequals(
            it->name, name))
            break;
        ++it;
    }
    return it;
}

auto
headers::
matching(field id) const noexcept ->
    subrange
{
    return subrange(
        this, find(id).i_);
}

auto
headers::
matching(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        this, find(name).i_);
}

//------------------------------------------------

std::size_t
headers::
find(
    std::size_t after,
    field id) const noexcept
{
    std::size_t i = after;
    while(++i < count_)
        if((*this)[i].id == id)
            break;
    return i;
}

std::size_t
headers::
find(
    std::size_t after,
    string_view name) const noexcept
{
    std::size_t i = after;
    while(++i < count_)
        if(bnf::iequals(
            (*this)[i].name, name))
            break;
    return i;
}

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
    fields_bytes_ =
        dest - buf_ -
            prefix_bytes_;
    *dest++ = '\r';
    *dest++ = '\n';
    ++count_;
}

//------------------------------------------------

std::string
headers::
subrange::
make_list() const
{
    auto it = begin();
    auto const end_ = end();
    std::string s;
    if(it == end_)
        return s;
    s.append(
        it->value.data(),
        it->value.size());
    while(++it != end_)
    {
        s.push_back(',');
        s.append(it->value.data(),
            it->value.size());
    }
    return s;
}

auto
headers::
subrange::
iterator::
operator++() noexcept ->
    iterator&
{
    BOOST_ASSERT(
        i_ < h_->size());
    if((*h_)[i_].id !=
        field::unknown)
    {
        i_ = h_->find(i_,
            (*h_)[i_].id);
    }
    else
    {
        i_ = h_->find(i_,
            (*h_)[i_].name);
    }
    return *this;
}

} // http_proto
} // boost

#endif
