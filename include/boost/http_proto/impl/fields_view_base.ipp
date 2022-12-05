//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_BASE_IPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_BASE_IPP

#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/assert/source_location.hpp>
#include <utility>

namespace boost {
namespace http_proto {

fields_view_base::
value_type::
value_type(
    reference const& other)
    : id(other.id)
    , name(other.name)
    , value(other.value)
{
}

//------------------------------------------------

auto
fields_view_base::
iterator::
operator*() const noexcept ->
    reference
{
    auto const& e =
        ph_->tab()[i_];
    auto const* p =
        ph_->cbuf + ph_->prefix;
    return {
        e.id,
        string_view(
            p + e.np, e.nn),
        string_view(
            p + e.vp, e.vn) };
}

//------------------------------------------------

fields_view_base::
subrange::
iterator::
iterator(
    detail::header const* ph,
    std::size_t i) noexcept
    : ph_(ph)
    , i_(i)
{
    if(i_ >= ph_->count)
        return;
    auto const* e = &ph_->tab()[i_];
    auto const id = e->id;
    switch(id)
    {
    case field::connection:
        n_ = ph_->md.connection.count;
        break;
    case field::content_length:
        n_ = ph_->md.content_length.count;
        break;
    case field::transfer_encoding:
        n_ = ph_->md.transfer_encoding.count;
        break;
    case field::upgrade:
        n_ = ph_->md.upgrade.count;
        break;
    default:
        break;
    }
}

auto
fields_view_base::
subrange::
iterator::
operator*() const noexcept ->
    reference const
{
    auto const& e =
        ph_->tab()[i_];
    auto const p =
        ph_->cbuf + ph_->prefix;
    return string_view(
        p + e.vp, e.vn);
}

auto
fields_view_base::
subrange::
iterator::
operator++() noexcept ->
    iterator&
{
    BOOST_ASSERT(i_ < ph_->count);
    BOOST_ASSERT(n_ > 0);
    if(--n_ == 0)
    {
        i_ = ph_->count;
        return *this;
    }
    auto const* e = &ph_->tab()[i_];
    auto const id = e->id;
    if(id != field::unknown)
    {
        ++i_;
        --e;
        while(i_ != ph_->count)
        {
            if(e->id == id)
                break;
            ++i_;
            --e;
        }
        return *this;
    }
    auto const p =
        ph_->cbuf + ph_->prefix;
    auto name =
        string_view(p + e->np, e->nn);
    ++i_;
    --e;
    while(i_ != ph_->count)
    {
        if(grammar::ci_is_equal(
            name, string_view(
                p + e->np, e->nn)))
            break;
        ++i_;
        --e;
    }
    return *this;
}

//------------------------------------------------
//
// fields_view_base
//
//------------------------------------------------

bool
fields_view_base::
exists(
    field id) const noexcept
{
    return find(id) != end();
}

bool
fields_view_base::
exists(
    string_view name) const noexcept
{
    return find(name) != end();
}

std::size_t
fields_view_base::
count(field id) const noexcept
{
    std::size_t n = 0;
    for(auto const& v : *this)
        if(v.id == id)
            ++n;
    return n;
}

std::size_t
fields_view_base::
count(string_view name) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(grammar::ci_is_equal(
                v.name, name))
            ++n;
    return n;
}

auto
fields_view_base::
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
fields_view_base::
find(string_view name) const noexcept ->
    iterator
{
    auto it = begin();
    auto const last = end();
    while(it != last)
    {
        if(grammar::ci_is_equal(
                it->name, name))
            break;
        ++it;
    }
    return it;
}

auto
fields_view_base::
find(
    iterator from,
    field id) const noexcept ->
        iterator
{
    auto const last = end();
    while(from != last)
    {
        if(from->id == id)
            break;
        ++from;
    }
    return from;
}

auto
fields_view_base::
find(
    iterator from, 
    string_view name) const noexcept ->
        iterator
{
    auto const last = end();
    while(from != last)
    {
        if(grammar::ci_is_equal(
                name, from->name))
            break;
        ++from;
    }
    return from;
}

auto
fields_view_base::
find_last(
    iterator it,
    field id) const noexcept ->
        iterator
{
    auto const it0 = begin();
    for(;;)
    {
        if(it == it0)
            return end();
        --it;
        if(it->id == id)
            return it;
    }
}

auto
fields_view_base::
find_last(
    iterator it,
    string_view name) const noexcept ->
        iterator
{
    auto const it0 = begin();
    for(;;)
    {
        if(it == it0)
            return end();
        --it;
        if(grammar::ci_is_equal(
                it->name, name))
            return it;
    }
}

auto
fields_view_base::
find_all(
    field id) const noexcept ->
        subrange
{
    return subrange(
        ph_, find(id).i_);
}

auto
fields_view_base::
find_all(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        ph_, find(name).i_);
}

//------------------------------------------------

} // http_proto
} // boost

#endif
