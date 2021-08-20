//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_HTTP_PROTO_IMPL_HEADERS_VIEW_IPP
#define BOOST_HTTP_PROTO_IMPL_HEADERS_VIEW_IPP

#include <boost/http_proto/headers_view.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/ftab.hpp>
#include <utility>

namespace boost {
namespace http_proto {

auto
headers_view::
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
headers_view::
exists(
    field id) const noexcept
{
    return find(id) != end();
}

bool
headers_view::
exists(
    string_view name) const noexcept
{
    return find(name) != end();
}

std::size_t
headers_view::
count(field id) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(v.id == id)
            ++n;
    return n;
}

std::size_t
headers_view::
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
headers_view::
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
headers_view::
at(field id) const
{
    auto it = find(id);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found", BOOST_CURRENT_LOCATION);
}

string_view
headers_view::
at(string_view name) const
{
    auto it = find(name);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found", BOOST_CURRENT_LOCATION);
}

string_view
headers_view::
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
headers_view::
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
headers_view::
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
headers_view::
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
headers_view::
matching(field id) const noexcept ->
    subrange
{
    return subrange(
        this, find(id).i_);
}

auto
headers_view::
matching(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        this, find(name).i_);
}

//------------------------------------------------

std::size_t
headers_view::
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
headers_view::
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

//------------------------------------------------

std::string
headers_view::
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
headers_view::
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
