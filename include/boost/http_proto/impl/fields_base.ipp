//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_BASE_IPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_BASE_IPP

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/detail/copied_strings.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/assert.hpp>
#include <boost/assert/source_location.hpp>
#include <string>

namespace boost {
namespace http_proto {

fields_base::
~fields_base()
{
    if(h_.buf)
        delete[] h_.buf;
}

fields_base::
fields_base(
    detail::header const& h) noexcept
    : fields_view_base(h)
{
}

fields_base::
fields_base(
    detail::kind k) noexcept
    : fields_view_base(k)
{
}

// copy with start-line
fields_base::
fields_base(
    fields_view_base const& fv,
    detail::kind k)
    : fields_base(
    [&fv, k]
    {
        BOOST_ASSERT(k == fv.h_.kind);
        detail::header h = fv.h_;
        if(! is_default(h.cbuf))
        {
            // copy start line and fields
            auto n = detail::buffer_needed(
                fv.h_.size, fv.h_.count);
            BOOST_ASSERT(n < max_off_t);
            h.buf = new char[n];
            std::memcpy(
                h.buf, fv.h_.cbuf, fv.h_.size);
            fv.write_table(h.buf + n);
            h.cbuf = h.buf;
            h.cap = n;
            return h;
        }

        // default buffer
        BOOST_ASSERT(h.cap == 0);
        BOOST_ASSERT(h.buf == nullptr);
        return h;
    }())
{
}

// copy everything
void
fields_base::
copy(fields_view_base const& other)
{
    if(! is_default(other.h_.cbuf))
    {
        auto n = detail::buffer_needed(
            other.h_.size, other.h_.count);
        if(n <= h_.cap)
        {
            // copy start line and fields
            other.write_table(h_.buf + h_.size);
            std::memcpy(
                h_.buf,
                other.h_.cbuf,
                other.h_.size);
            h_.prefix = other.h_.prefix;
            h_.size = other.h_.size;
            h_.count = other.h_.count;
            return;
        }
    }
    fields_base tmp(other, h_.kind);
    tmp.swap(*this);
}

//------------------------------------------------
//
// Modifiers
//
//------------------------------------------------

void
fields_base::
reserve(std::size_t n)
{
    if(n > max_off_t)
        detail::throw_length_error(
            " n > max off_t",
            BOOST_CURRENT_LOCATION);
    // align up
    n = detail::buffer_needed(n, 0);
    if(n <= h_.cap)
        return;
    auto buf = new char[n];
    if(h_.cap > 0)
    {
        std::memcpy(
            buf,
            h_.cbuf,
            h_.size);
        write_table(buf + n);
        delete[] h_.buf;
    }
    else
    {
        // default buffer
        std::memcpy(
            buf,
            h_.cbuf,
            h_.size);
    }
    h_.cap = n;
    h_.cbuf = buf;
    h_.buf = buf;
}

void
fields_base::
shrink_to_fit() noexcept
{
    if(detail::buffer_needed(
        h_.size, h_.count) >=
            h_.cap)
        return;
    fields_base(*this,
        detail::kind::fields).swap(*this);
}

void
fields_base::
emplace_back(
    field id,
    string_view value)
{
    insert_impl(
        id,
        to_string(id),
        value,
        h_.count);
}

void
fields_base::
emplace_back(
    string_view name,
    string_view value)
{
    insert_impl(
        string_to_field(
            name),
        name,
        value,
        h_.count);
}

auto
fields_base::
emplace(
    iterator before,
    field id,
    string_view value) ->
        iterator
{
    insert_impl(
        id,
        to_string(id),
        value,
        before.i_);
    return before;
}

auto
fields_base::
emplace(
    iterator before,
    string_view name,
    string_view value) ->
        iterator
{
    insert_impl(
        string_to_field(name),
        name,
        value,
        before.i_);
    return before;
}

auto
fields_base::
erase(iterator it) noexcept ->
    iterator
{
    BOOST_ASSERT(it.i_ < h_.count);
    BOOST_ASSERT(h_.buf != nullptr);
    detail::fields_table ft(
        h_.buf + h_.cap);
    auto const p0 =
        offset(ft, it.i_);
    auto const p1 =
        offset(ft, it.i_ + 1);
    std::memmove(
        h_.buf + p0,
        h_.buf + p1,
        h_.size - p1);
    auto const n = p1 - p0;
    --h_.count;
    for(std::size_t i = it.i_;
            i < h_.count; ++i)
        ft[i] = ft[i + 1] - n;
    h_.size = static_cast<
        off_t>(h_.size - n);
    return iterator(this, it.i_);
}

std::size_t
fields_base::
erase(
    field id) noexcept
{
    auto it = find(id);
    if(it != end())
    {
        erase(it);
        return 1;
    }
    return 0;
}

std::size_t
fields_base::
erase(
    string_view name) noexcept
{
    auto it = find(name);
    if(it != end())
    {
        erase(it);
        return 1;
    }
    return 0;
}

std::size_t
fields_base::
erase_all(
    field id) noexcept
{
    std::size_t n = 0;
    std::size_t i = h_.count;
    while(i > 0)
    {
        --i;
        iterator it(this, i);
        if(it->id != id)
            continue;
        erase(it);
        ++n;
    }
    return n;
}

std::size_t
fields_base::
erase_all(
    string_view name) noexcept
{
    // find the first occurrence and use
    // its string instead, to handle the
    // case where `name` comes from the
    // fields buffer.
    auto it0 = find(name);
    if(it0 == end())
        return 0;
    name = it0->name;
    std::size_t n = 1;
    std::size_t i = h_.count - 1;
    while(i != it0.i_)
    {
        iterator it(this, i);
        if(bnf::iequals(
            it->name, name))
        {
            erase(it);
            ++n;
        }
        --i;
    }
    erase(it0);
    return n;
}

//------------------------------------------------
//
// (implementation)
//
//------------------------------------------------

// return i-th field absolute offset
std::size_t
fields_base::
offset(
    detail::fields_table const& ft,
    std::size_t i) const noexcept
{
    if(i < h_.count)
        return h_.prefix + ft[i].np;
    // make final CRLF the last "field"
    BOOST_ASSERT(i == h_.count);
    return h_.size - 2;
}

void
fields_base::
insert_impl(
    field id,
    string_view name,
    string_view value,
    std::size_t before)
{
    BOOST_ASSERT(before <= h_.count);
    auto const n0 =
        name.size() + 2 +
        value.size() + 2;
    auto const n1 = h_.size + n0;
    if(n1 > max_off_t)
        detail::throw_length_error(
            "too large",
            BOOST_CURRENT_LOCATION);
    auto const n =
        detail::buffer_needed(
            n1, h_.count + 1);
    if(h_.cap >= n)
    {
        BOOST_ASSERT(h_.buf != nullptr);
        detail::copied_strings cs(
            string_view(h_.cbuf, h_.size));
        name = cs.maybe_copy(name);
        value = cs.maybe_copy(value);
        detail::fields_table ft(
            h_.buf + h_.cap);
        auto pos = offset(ft, before);
        char* dest = h_.buf + pos;
        std::memmove(
            dest + n0,
            dest,
            h_.size - pos);
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        *dest++ = ' ';
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';
        for(std::size_t i = h_.count;
            i > before; --i)
            ft[i] = ft[i-1] + n0;
        pos -= h_.prefix;
        auto& e = ft[before];
        e.np = static_cast<
            off_t>(pos);
        e.nn = static_cast<
            off_t>(name.size());
        e.vp = static_cast<off_t>(
            pos + name.size() + 2);
        e.vn = static_cast<
            off_t>(value.size());
        e.id = id;
        h_.size = static_cast<
            off_t>(h_.size + n0);
        ++h_.count;
    }
    else if(h_.cap > 0)
    {
        // grow
        BOOST_ASSERT(h_.buf != nullptr);
        auto buf = new char[n];

        // copy existing fields
        // and write new field
        detail::fields_table ft0(
            h_.buf + h_.cap);
        auto pos = offset(ft0, before);
        char* dest = buf;
        std::memcpy(
            dest,
            h_.buf,
            pos);
        dest += pos;
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        *dest++ = ' ';
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';
        std::memcpy(
            dest,
            h_.buf + pos,
            h_.size - pos);

        // write table
        ft0.copy(buf + n, before);
        detail::fields_table ft(buf + n);
        for(auto i = before;
            i < h_.count; ++i)
            ft[i + 1] = ft0[i] + n0;

        auto& e = ft[before];
        pos -= h_.prefix;
        e.np = static_cast<
            off_t>(pos);
        e.nn = static_cast<
            off_t>(name.size());
        e.vp = static_cast<off_t>(
            pos + name.size() + 2);
        e.vn = static_cast<
            off_t>(value.size());
        e.id = id;

        delete[] h_.buf;
        h_.buf = buf;
        h_.cbuf = buf;
        h_.cap = n;
        h_.size = static_cast<
            off_t>(n1);
        h_.count += 1;
    }
    else
    {
        BOOST_ASSERT(h_.buf == nullptr);
        BOOST_ASSERT(before == 0);
        BOOST_ASSERT(h_.count == 0);

        // initial allocation
        auto s = default_buffer(h_.kind);
        h_.buf = new char[n];
        h_.cbuf = h_.buf;
        h_.cap = n;
        h_.prefix = static_cast<
            off_t>(s.size() - 2);
        h_.size = static_cast<
            off_t>(n1);
        h_.count = 1;

        // start line
        char* dest = h_.buf;
        s.copy(dest, h_.prefix);
        dest += h_.prefix;

        // write field
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        *dest++ = ' ';
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';

        *dest++ = '\r';
        *dest++ = '\n';

        // write table
        detail::fields_table ft(
            h_.buf + h_.cap);
        auto& e = ft[before];
        e.id = id;
        e.np = 0;
        e.nn = static_cast<
            off_t>(name.size());
        e.vp = static_cast<
            off_t>(name.size() + 2);
        e.vn = static_cast<
            off_t>(value.size());
    }
}

//------------------------------------------------

void
fields_base::
swap(fields_base& other) noexcept
{
    this->fields_view_base::swap(other);
}


void
fields_base::
clear() noexcept
{
    if(! h_.buf)
        return;
    auto s =
        default_buffer(h_.kind);
    h_.size = static_cast<
        off_t>(s.size());
    h_.prefix = h_.size - 2;
    h_.count = 0;
    std::memcpy(
        h_.buf,
        s.data(),
        h_.size);
}

char*
fields_base::
set_start_line_impl(
    std::size_t n)
{
    if( n > h_.prefix ||
        h_.buf == nullptr)
    {
        // allocate or grow
        if( n > h_.prefix &&
            static_cast<std::size_t>(
                n - h_.prefix) >
            static_cast<std::size_t>(
                max_off_t - h_.size))
            detail::throw_length_error(
                "too large",
                BOOST_CURRENT_LOCATION);
        auto n0 = detail::buffer_needed(
            n + h_.size - h_.prefix,
            h_.count);
        auto buf = new char[n0];
        if(h_.buf != nullptr)
        {
            std::memcpy(
                buf + n,
                h_.buf + h_.prefix,
                h_.size - h_.prefix);
            detail::fields_table ft(
                h_.buf + h_.cap);
            ft.copy(buf + n0, h_.count);
            delete[] h_.buf;
        }
        else
        {
            std::memcpy(
                buf + n,
                h_.cbuf + h_.prefix,
                h_.size - h_.prefix);
        }
        h_.buf = buf;
        h_.cbuf = buf;
        h_.size = static_cast<
            off_t>(h_.size +
                n - h_.prefix);
        h_.prefix = static_cast<
            off_t>(n);
        h_.cap = n0;
        return h_.buf;
    }

    // shrink
    std::memmove(
        h_.buf + n,
        h_.buf + h_.prefix,
        h_.size - h_.prefix);
    h_.size = static_cast<
        off_t>(h_.size -
            h_.prefix + n);
    h_.prefix = static_cast<
        off_t>(n);
    return h_.buf;
}

} // http_proto
} // boost

#endif
