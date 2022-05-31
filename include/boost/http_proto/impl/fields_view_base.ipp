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
    if(i_ >= f_->h_.count)
    {
        it_ =
            f_->h_.cbuf +
            f_->h_.size;
        return;
    }

    if(f_->h_.cap != 0)
    {
        // use field table
        return;
    }

    error_code ec;
    field_rule r;
    auto const end = 
        f_->h_.cbuf + f_->h_.size;
    grammar::parse(it_, end, ec, r);
    BOOST_ASSERT(! ec.failed());
    auto const base =
        f_->h_.cbuf + f_->h_.prefix;
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
    , it_(f->h_.cbuf + f->h_.prefix)
    , i_(static_cast<
        off_t>(i))
{
    BOOST_ASSERT(
        f->h_.cap != 0 || (
            i == 0 ||
            i == f->h_.count));
    read();
}

auto
fields_view_base::
iterator::
operator*() const noexcept ->
    reference
{
    auto const base =
        f_->h_.cbuf + f_->h_.prefix;
    if(f_->h_.cap != 0)
    {
        // use field table
        auto const& e =
            detail::const_fields_table(
                f_->h_.cbuf + f_->h_.cap)[i_];
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
    detail::kind k) noexcept
{
    switch(k)
    {
    default:
    case detail::kind::fields: return {
        "\r\n", 2 };
    case detail::kind::request: return {
        "GET / HTTP/1.1\r\n\r\n", 18 };
    case detail::kind::response: return {
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
        s == default_buffer(
            detail::kind::fields).data() ||
        s == default_buffer(
            detail::kind::request).data() ||
        s == default_buffer(
            detail::kind::response).data();
}

// copy or build table
void
fields_view_base::
write_table(
    void* dest) const noexcept
{
    if(h_.cap > 0)
    {
        detail::const_fields_table(
            h_.cbuf + h_.cap).copy(
                dest, h_.count);
        return;
    }

    auto it = begin();
    auto const end = this->end();
    std::size_t i = 0;
    detail::fields_table ft(dest);
    auto const base =
        h_.cbuf + h_.prefix;
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
    h_.swap(other.h_);
}

fields_view_base::
fields_view_base(
    detail::header const& h) noexcept
    : h_(h)
{
    BOOST_ASSERT(
        h_.cap == 0 ||
        h_.cap >= h_.size);
    BOOST_ASSERT(
        h_.size >= h_.prefix);
    BOOST_ASSERT(
        h_.prefix <= max_off_t);
    BOOST_ASSERT(
        h_.size <= max_off_t);
    BOOST_ASSERT(
        h_.count <= max_off_t);

    switch(h_.kind)
    {
    case detail::kind::fields:
        break;
    case detail::kind::request:
        BOOST_ASSERT(
            h_.req.method_len <= max_off_t);
        BOOST_ASSERT(
            h_.req.target_len <= max_off_t);
        break;
    case detail::kind::response:
        break;
    }
}

fields_view_base::
fields_view_base(
    detail::kind k) noexcept
    : fields_view_base(
    [k]
    {
        auto s =
            default_buffer(k);
        detail::header h;
        h.kind = k;
        h.cbuf = s.data();
        h.cap = 0;
        h.prefix = static_cast<
            off_t>(s.size() - 2);
        h.size = h.prefix + 2;
        h.count = 0;
        switch(h.kind)
        {
        case detail::kind::fields:
            break;
        case detail::kind::request:
            h.req.method_len = 3;
            h.req.target_len = 1;
            h.req.method =
                http_proto::method::get;
            h.req.version =
                http_proto::version::http_1_1;
            break;
        case detail::kind::response:
            h.res.version =
                http_proto::version::http_1_1;
            h.res.status =
                http_proto::status::ok;
            h.res.status_int = 200;
            break;
        }
        return h;
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
        this, h_.count);
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

} // http_proto
} // boost

#endif
