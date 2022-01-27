//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_IPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_IPP

#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/assert/source_location.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

void
fields_view::
iterator::
read() noexcept
{
#if 0
    if(! t_.empty())
    {
        auto e = t_(it_, i_);
        n_ = e.name;
        v_ = e.value;
        id_ = e.id;
        return;
    }
#endif
    error_code ec;
    field_rule r;
    if(grammar::parse(
        it_, end_, ec, r))
    {
        n_ = r.v.name;
        v_ = r.v.value;
        id_ = string_to_field(n_);
        return;
    }
    auto s = ec.message();
    BOOST_ASSERT(ec ==
        grammar::error::end);
    it_ = end_;
}

fields_view::
iterator::
iterator(
    fields_view const* f,
    detail::const_fields_table t) noexcept
    : it_( f->base_ + f->start_len_)
    , end_(f->base_ + f->end_len_)
    , t_(t)
{
    read();
}

fields_view::
iterator::
iterator(
    fields_view const* f,
    detail::const_fields_table t,
    int) noexcept
    : it_( f->base_ + f->end_len_)
    , end_(f->base_ + f->end_len_)
    , t_(t)
    , i_(f->count_)
{
}

auto
fields_view::
iterator::
operator*() const noexcept ->
    reference
{
    return { id_, n_, v_ };
}

auto
fields_view::
iterator::
operator++() noexcept ->
    iterator&
{
    ++i_;
    read();
    return *this;
}

//------------------------------------------------

fields_view::
fields_view(
    ctor_params const& init) noexcept
    : base_(init.base)
    , t_(init.table)
    , start_len_(static_cast<
        off_t>(init.start_len))
    , end_len_(static_cast<
        off_t>(init.end_len))
    , count_(static_cast<
        off_t>(init.count))
{
    BOOST_ASSERT(start_len_ <=
        BOOST_HTTP_PROTO_MAX_HEADER);
    BOOST_ASSERT(end_len_ <=
        BOOST_HTTP_PROTO_MAX_HEADER);
    BOOST_ASSERT(count_ <=
        BOOST_HTTP_PROTO_MAX_HEADER);
}

fields_view::
fields_view(
    fields_view const&) noexcept = default;

fields_view&
fields_view::
operator=(
    fields_view const&) noexcept = default;

fields_view::
fields_view() noexcept = default;

fields_view::
fields_view(string_view s)
    : fields_view(
    [&s]
    {
        ctor_params init;
        init.base = s.data();
        init.start_len = 0;
        init.end_len = s.size();
        init.count = 0;
        init.table = nullptr;
        char const* it = s.data();
        char const* const end =
            s.data() + s.size();
        error_code ec;
        field_rule r;
        for(;;)
        {
            if(grammar::parse(
                it, end, ec, r))
            {
                ++init.count;
                continue;
            }
            if(ec == grammar::error::end)
                break;
            detail::throw_system_error(ec,
                BOOST_CURRENT_LOCATION);
        }
        return init;
    }())
{
}

auto
fields_view::
begin() const noexcept ->
    iterator
{
    return iterator(this, t_);
}

auto
fields_view::
end() const noexcept ->
    iterator
{
    return iterator(this, t_, 0);
}

string_view
fields_view::
get_const_buffer() const noexcept
{
    return string_view(
        base_, end_len_);
}

//------------------------------------------------

bool
fields_view::
exists(
    field id) const noexcept
{
    return find(id) != end();
}

bool
fields_view::
exists(
    string_view name) const noexcept
{
    return find(name) != end();
}

std::size_t
fields_view::
count(field id) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(v.id == id)
            ++n;
    return n;
}

std::size_t
fields_view::
count(string_view name) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(bnf::iequals(
            v.name, name))
            ++n;
    return n;
}

string_view
fields_view::
at(field id) const
{
    auto it = find(id);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found",
        BOOST_CURRENT_LOCATION);
}

string_view
fields_view::
at(string_view name) const
{
    auto it = find(name);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found",
        BOOST_CURRENT_LOCATION);
}

string_view
fields_view::
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
fields_view::
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
fields_view::
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
fields_view::
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
fields_view::
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
fields_view::
find(
    iterator from, 
    string_view name) const noexcept ->
        iterator
{
    auto const last = end();
    while(from != last)
    {
        if( bnf::iequals(
            name, from->name))
            break;
        ++from;
    }
    return from;
}

auto
fields_view::
find_all(field id) const noexcept ->
    subrange
{
    return subrange(
        find(id), end());
}

auto
fields_view::
find_all(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        find(name), end());
}

//------------------------------------------------

fields_view::
subrange::
subrange(
    fields_view::iterator it,
    fields_view::iterator end) noexcept
    : it_(it)
    , end_(end)
    , id_(it == end
        ? field::unknown
        : it->id)
{
}

fields_view::
subrange::
subrange(
    subrange const&) noexcept = default;

auto
fields_view::
subrange::
operator=(
    subrange const&) noexcept ->
        subrange& = default;

fields_view::
subrange::
subrange() noexcept = default;

auto
fields_view::
subrange::
begin() const noexcept ->
    iterator
{
    return subrange::iterator(
        it_, end_, id_);
}

auto
fields_view::
subrange::
end() const noexcept ->
    iterator
{
    return subrange::iterator(
        end_, end_, id_);
}

//------------------------------------------------

fields_view::
subrange::
iterator::
iterator(
    fields_view::iterator it,
    fields_view::iterator end,
    field id) noexcept
    : it_(it)
    , end_(end)
    , id_(id)
{
}

auto
fields_view::
subrange::
iterator::
operator++() noexcept ->
    iterator&
{
    if(id_ != field::unknown)
    {
        ++it_;
        while(it_ != end_)
        {
            if(it_->id == id_)
                break;
            ++it_;
        }
        return *this;
    }
    string_view name =
        it_->name;
    ++it_;
    while(it_ != end_)
    {
        if(bnf::iequals(
            name, it_->name))
            break;
        ++it_;
    }
    return *this;
}

} // http_proto
} // boost

#endif
