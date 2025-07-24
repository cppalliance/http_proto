//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/fields_view_base.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/assert.hpp>
#include <boost/assert/source_location.hpp>

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
    BOOST_ASSERT(i_ < ph_->count);
    auto tab =
        ph_->tab();
    auto const& e =
        tab[i_];
    auto const* p =
        ph_->cbuf + ph_->prefix;
    return {
        (e.id == detail::header::unknown_field)
            ? optional<field>{} : e.id,
        core::string_view(
            p + e.np, e.nn),
        core::string_view(
            p + e.vp, e.vn) };
}

//------------------------------------------------

auto
fields_view_base::
reverse_iterator::
operator*() const noexcept ->
    reference
{
    BOOST_ASSERT(i_ > 0);
    auto tab =
      ph_->tab();
    auto const& e =
        tab[i_-1];
    auto const* p =
        ph_->cbuf + ph_->prefix;
    return {
        (e.id == detail::header::unknown_field)
            ? optional<field>{} : e.id,
        core::string_view(
            p + e.np, e.nn),
        core::string_view(
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
    BOOST_ASSERT(i <= ph_->count);
}

fields_view_base::
subrange::
iterator::
iterator(
    detail::header const* ph) noexcept
    : ph_(ph)
    , i_(ph->count)
{
}

auto
fields_view_base::
subrange::
iterator::
operator*() const noexcept ->
    reference const
{
    auto tab =
        ph_->tab();
    auto const& e =
        tab[i_];
    auto const p =
        ph_->cbuf + ph_->prefix;
    return core::string_view(
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
    auto const* e = &ph_->tab()[i_];
    auto const id = e->id;
    if(id != detail::header::unknown_field)
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
    auto name = core::string_view(
        p + e->np, e->nn);
    ++i_;
    --e;
    while(i_ != ph_->count)
    {
        if(grammar::ci_is_equal(
            name, core::string_view(
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

core::string_view
fields_view_base::
at(
    field id) const
{
    auto const it = find(id);
    if(it == end())
        BOOST_THROW_EXCEPTION(
            std::out_of_range{ "field not found" });
    return it->value;
}

core::string_view
fields_view_base::
at(
    core::string_view name) const
{
    auto const it = find(name);
    if(it == end())
        BOOST_THROW_EXCEPTION(
            std::out_of_range{ "field not found" });
    return it->value;
}

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
    core::string_view name) const noexcept
{
    return find(name) != end();
}

std::size_t
fields_view_base::
count(field id) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(v.id == id)
            ++n;
    return n;
}

std::size_t
fields_view_base::
count(
    core::string_view name) const noexcept
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
find(
    core::string_view name) const noexcept ->
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
    core::string_view name) const noexcept ->
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
    core::string_view name) const noexcept ->
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

core::string_view
fields_view_base::
value_or(
    field id,
    core::string_view s) const noexcept
{
    auto it = find(id);
    if(it != end())
        return it->value;
    return s;
}

core::string_view
fields_view_base::
value_or(
    core::string_view name,
    core::string_view s) const noexcept
{
    auto it = find(name);
    if(it != end())
        return it->value;
    return s;
}

//------------------------------------------------

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
    core::string_view name) const noexcept ->
        subrange
{
    return subrange(
        ph_, find(name).i_);
}

//------------------------------------------------

} // http_proto
} // boost
