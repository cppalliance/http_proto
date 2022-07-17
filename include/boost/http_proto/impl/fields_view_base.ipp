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
#include <boost/url/grammar/parse.hpp>
#include <boost/assert/source_location.hpp>
#include <utility>

namespace boost {
namespace http_proto {

//------------------------------------------------

auto
fields_view_base::
iterator::
operator*() const noexcept ->
    reference const
{
    auto const& e =
        h_->tab()[i_];
    auto const* p =
        h_->cbuf + h_->prefix;
    return {
        e.id,
        string_view(
            p + e.np, e.nn),
        string_view(
            p + e.vp, e.vn) };
}

//------------------------------------------------

auto
fields_view_base::
subrange::
iterator::
operator*() const noexcept ->
    reference const
{
    auto const& e =
        h_->tab()[i_];
    auto const p =
        h_->cbuf + h_->prefix;
    return {
        e.id,
        string_view(
            p + e.np, e.nn),
        string_view(
            p + e.vp, e.vn)};
}

auto
fields_view_base::
subrange::
iterator::
operator++() noexcept ->
    iterator&
{
    auto const* e = &h_->tab()[i_];
    auto const id = e->id;
    if(id != field::unknown)
    {
        ++i_;
        --e;
        while(i_ != h_->count)
        {
            if(e->id == id)
                break;
            ++i_;
        }
        return *this;
    }
    auto const p = h_->cbuf + h_->prefix;
    auto name =
        string_view(p + e->np, e->nn);
    ++i_;
    --e;
    while(i_ != h_->count)
    {
        if(bnf::iequals(name, string_view(
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

void
fields_view_base::
swap(fields_view_base& other) noexcept
{
    auto& h = other.h_;
    std::swap(h_.cbuf, h.cbuf);
    std::swap(h_.buf, h.buf);
    std::swap(h_.cap, h.cap);
    std::swap(h_.size, h.size);
    std::swap(h_.count, h.count);
    std::swap(h_.prefix, h.prefix);
    std::swap(h_.version, h.version);
    switch(h_.kind)
    {
    case detail::kind::fields:
        break;
    case detail::kind::request:
        std::swap(
            h_.req.method_len, h.req.method_len);
        std::swap(
            h_.req.target_len, h.req.target_len);
        std::swap(h_.req.method, h.req.method);
        break;
    case detail::kind::response:
        std::swap(
            h_.res.status_int, h.res.status_int);
        std::swap(h_.res.status, h.res.status);
        break;
    }
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
        auto s = default_buffer(k);
        detail::header h(k);
        h.cbuf = s.data();
        h.cap = 0;
        h.prefix = static_cast<
            off_t>(s.size() - 2);
        h.size = h.prefix + 2;
        h.count = 0;
        h.version =
            http_proto::version::http_1_1;
        switch(h.kind)
        {
        case detail::kind::fields:
            break;
        case detail::kind::request:
            h.req.method_len = 3;
            h.req.target_len = 1;
            h.req.method =
                http_proto::method::get;
            break;
        case detail::kind::response:
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
find_all(
    field id) const noexcept ->
        subrange
{
    return subrange(
        &h_, find(id).i_);
}

auto
fields_view_base::
find_all(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        &h_, find(name).i_);
}

//------------------------------------------------

} // http_proto
} // boost

#endif
