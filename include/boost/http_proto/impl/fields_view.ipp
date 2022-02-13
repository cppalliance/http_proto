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
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/assert/source_location.hpp>
#include <utility>

namespace boost {
namespace http_proto {

//------------------------------------------------

fields_view_base::
subrange::
subrange(
    fields_view_base::iterator it,
    fields_view_base::iterator end) noexcept
    : it_(it)
    , end_(end)
    , id_(it == end
        ? field::unknown
        : it->id)
{
}

fields_view_base::
subrange::
subrange(
    subrange const&) noexcept = default;

auto
fields_view_base::
subrange::
operator=(
    subrange const&) noexcept ->
        subrange& = default;

fields_view_base::
subrange::
subrange() noexcept = default;

auto
fields_view_base::
subrange::
begin() const noexcept ->
    iterator
{
    return subrange::iterator(
        it_, end_, id_);
}

auto
fields_view_base::
subrange::
end() const noexcept ->
    iterator
{
    return subrange::iterator(
        end_, end_, id_);
}

//------------------------------------------------

fields_view_base::
subrange::
iterator::
iterator(
    fields_view_base::iterator it,
    fields_view_base::iterator end,
    field id) noexcept
    : it_(it)
    , end_(end)
    , id_(id)
{
}

auto
fields_view_base::
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

//------------------------------------------------

void
fields_view_base::
iterator::
read() noexcept
{
    if(i_ >= f_->count_)
    {
        it_ =
            f_->cbuf_ +
            f_->end_pos_;
        return;
    }

    if(f_->buf_len_ != 0)
    {
        // use field table
        return;
    }

    error_code ec;
    field_rule r;
    auto const end = 
        f_->cbuf_ + f_->end_pos_;
    grammar::parse(it_, end, ec, r);
    BOOST_ASSERT(! ec.failed());
    auto const base =
        f_->cbuf_ + f_->start_len_;
    np_ = static_cast<off_t>(
        r.v.name.data() - base);
    nn_ = static_cast<off_t>(
        r.v.name.size());
    vp_ = static_cast<off_t>(
        r.v.value.data() - base);
    vn_ = static_cast<off_t>(
        r.v.value.size());
    id_ = string_to_field(
        r.v.name);
}

fields_view_base::
iterator::
iterator(
    fields_view_base const* f,
    std::size_t i) noexcept
    : f_(f)
    , it_(f->cbuf_ + f->start_len_)
    , i_(static_cast<
        off_t>(i))
{
    BOOST_ASSERT(
        f->buf_len_ != 0 || (
            i == 0 ||
            i == f->count_));
    read();
}

auto
fields_view_base::
iterator::
operator*() const noexcept ->
    reference
{
    auto const base =
        f_->cbuf_ + f_->start_len_;
    if(f_->buf_len_ != 0)
    {
        // use field table
        auto const& e =
            detail::const_fields_table(
                f_->cbuf_ + f_->buf_len_)[i_];
        return {
            e.id,
            string_view(
                base + e.np,
                e.nn),
            string_view(
                base + e.vp,
                e.vn) };
    }
    return { id_,
        string_view(
            base + np_, nn_),
        string_view(
            base + vp_, vn_) };
}

auto
fields_view_base::
iterator::
operator++() noexcept ->
    iterator&
{
    ++i_;
    read();
    return *this;
}

//------------------------------------------------
//
// fields_view_base
//
//------------------------------------------------

// return default buffer for kind
// 0 = fields (no start-line)
// 1 = request
// 2 = response
string_view
fields_view_base::
default_buffer(
    char kind) noexcept
{
    switch(kind)
    {
    default:
    case 0: return {
        "\r\n", 2 };
    case 1: return {
        "GET / HTTP/1.1\r\n\r\n", 18 };
    case 2: return {
        "HTTP/1.1 200 OK\r\n\r\n", 19 };
    }
}

// return true if s is a default string
bool
fields_view_base::
is_default(
    char const* s) noexcept
{
    return
        s == default_buffer(0).data() ||
        s == default_buffer(1).data() ||
        s == default_buffer(2).data();
}

// copy or build table
void
fields_view_base::
write_table(
    void* dest) const noexcept
{
    if(buf_len_ > 0)
    {
        detail::const_fields_table(
            cbuf_ + buf_len_).copy(
                dest, count_);
        return;
    }

    auto it = begin();
    auto const end = this->end();
    std::size_t i = 0;
    detail::fields_table ft(dest);
    auto const base =
        cbuf_ + start_len_;
    while(it != end)
    {
        auto const v = *it;
        ft[i] = {
            static_cast<off_t>(
                v.name.data() - base),
            static_cast<off_t>(
                v.name.size()),
            static_cast<off_t>(
                v.value.data() - base),
            static_cast<off_t>(
                v.value.size()),
            v.id };
        ++it;
        ++i;
    }
}

void
fields_view_base::
swap(fields_view_base& other) noexcept
{
    using std::swap;
    swap(cbuf_, other.cbuf_);
    swap(buf_len_, other.buf_len_);
    swap(start_len_, other.start_len_);
    swap(end_pos_, other.end_pos_);
    swap(count_, other.count_);
}

fields_view_base::
fields_view_base(
    ctor_params const& init) noexcept
    : cbuf_(init.cbuf)
    , buf_len_(init.buf_len)
    , start_len_(static_cast<
        off_t>(init.start_len))
    , end_pos_(static_cast<
        off_t>(init.end_pos))
    , count_(static_cast<
        off_t>(init.count))
{
    BOOST_ASSERT(
        buf_len_ == 0 ||
        buf_len_ > end_pos_);
    BOOST_ASSERT(
        end_pos_ >= start_len_);
    BOOST_ASSERT(
        start_len_ <= max_off_t);
    BOOST_ASSERT(
        end_pos_ <= max_off_t);
    BOOST_ASSERT(
        count_ <= max_off_t);
}

fields_view_base::
fields_view_base(
    char kind) noexcept
    : fields_view_base(
    [kind]
    {
        auto s =
            default_buffer(kind);
        ctor_params init;
        init.cbuf = s.data();
        init.buf_len = 0;
        init.start_len = s.size() - 2;
        init.end_pos = s.size();
        init.count = 0;
        return init;
    }())
{
}

//------------------------------------------------

auto
fields_view_base::
begin() const noexcept ->
    iterator
{
    return iterator(
        this, 0);
}

auto
fields_view_base::
end() const noexcept ->
    iterator
{
    return iterator(
        this, count_);
}

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
    for(auto v : *this)
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
        if(bnf::iequals(
            v.name, name))
            ++n;
    return n;
}

string_view
fields_view_base::
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
fields_view_base::
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
fields_view_base::
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
fields_view_base::
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
        if(bnf::iequals(
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
        if( bnf::iequals(
            name, from->name))
            break;
        ++from;
    }
    return from;
}

auto
fields_view_base::
find_all(field id) const noexcept ->
    subrange
{
    return subrange(
        find(id), end());
}

auto
fields_view_base::
find_all(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        find(name), end());
}

//------------------------------------------------
//
// fields_view
//
//------------------------------------------------

fields_view::
fields_view(
    fields_view const&) noexcept = default;

fields_view&
fields_view::
operator=(
    fields_view const&) noexcept = default;

//------------------------------------------------

void
fields_view::
swap(fields_view& other) noexcept
{
    this->fields_view_base::swap(other);
}

} // http_proto
} // boost

#endif
