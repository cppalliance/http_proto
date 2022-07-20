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
#include <boost/http_proto/detail/number_string.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/assert.hpp>
#include <boost/assert/source_location.hpp>
#include <string>

namespace boost {
namespace http_proto {

fields_base::
fields_base(
    detail::kind k) noexcept
    : fields_view_base(&h_)
    , h_(k)
{
}

// construct a complete copy of h
fields_base::
fields_base(
    detail::header const& h)
    : fields_view_base(&h_)
    , h_(h)
{
    if(h_.is_default())
    {
        BOOST_ASSERT(h_.cap == 0);
        BOOST_ASSERT(h_.buf == nullptr);
        return;
    }

    // allocate and copy the buffer
    auto n = detail::buffer_needed(
        h.size, h.count);
    BOOST_ASSERT(n < max_off_t);
    h_.buf = new char[n];
    std::memcpy(
        h_.buf, h.cbuf, h.size);
    h.copy_table(h_.buf + n);
    h_.cbuf = h_.buf;
    h_.cap = n;
}

fields_base::
~fields_base()
{
    if(h_.buf)
        delete[] h_.buf;
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
        h_.copy_table(buf + n);
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
    h_.cbuf = buf;
    h_.buf = buf;
    h_.cap = n;
}

void
fields_base::
shrink_to_fit() noexcept
{
    if(detail::buffer_needed(
        h_.size, h_.count) >=
            h_.cap)
        return;
    fields_base tmp(h_);
    tmp.h_.swap(h_);
}

//------------------------------------------------

std::size_t
fields_base::
erase(
    field id) noexcept
{
    BOOST_ASSERT(
        id != field::unknown);
    auto const i = h_.find(id);
    if(i != h_.count)
        return erase_all_impl(i, id);
    return 0;
}

std::size_t
fields_base::
erase(
    string_view name) noexcept
{
    auto const i0 = h_.find(name);
    if(i0 == h_.count)
        return 0;
    auto const id = h_.tab()[i0].id;
    if(id == field::unknown)
        return raw_erase_all(i0);
    return erase_all_impl(i0, id);
}

//------------------------------------------------

// set the existing field to
// value, updating metadata.
void
fields_base::
set(
    iterator it,
    string_view value)
{
    // VFALCO copied_strings?
    detail::copied_strings cs(
        string_view(h_.cbuf, h_.size));
    value = cs.maybe_copy(value);

    auto const i = it.i_;
    auto const ft = h_.tab();
    auto const id = ft[i].id;
    {
        // provide strong guarantee
        auto const n0 =
            h_.size - length(ft, i);
        auto const n = ft[i].nn + 2 +
            value.size() + 2;
        // VFALCO missing overflow check
        reserve(n0 + n);
    }
    // VFALCO simple algorithm
    // but costs one extra memmove
    if(id != field::unknown)
    {
        erase_impl(i, id);
        // VFALCO This loses the original
        // capitalization of the field name
        insert_impl(id, to_string(id), value, i);
        return;
    }
    raw_set(i, value);
}

// erase existing fields with id
// and then add the field with value
void
fields_base::
set(
    field id,
    string_view value)
{
    BOOST_ASSERT(
        id != field::unknown);
    auto const i0 = h_.find(id);
    if(i0 != h_.count)
    {
        // field exists
        auto const ft = h_.tab();
        {
            // provide strong guarantee
            auto const n0 =
                h_.size - length(ft, i0);
            auto const n =
                ft[i0].nn + 2 +
                    value.size() + 2;
            // VFALCO missing overflow check
            reserve(n0 + n);
        }
        erase_all_impl(i0, id);
    }
    insert_impl(id, to_string(id),
        value, h_.count);
}

// erase existing fields with name
// and then add the field with value
void
fields_base::
set(
    string_view name,
    string_view value)
{
    auto const i0 = h_.find(name);
    if(i0 != h_.count)
    {
        // field exists
        auto const ft = h_.tab();
        auto const id = ft[i0].id;
        {
            // provide strong guarantee
            auto const n0 =
                h_.size - length(ft, i0);
            auto const n =
                ft[i0].nn + 2 +
                    value.size() + 2;
            // VFALCO missing overflow check
            reserve(n0 + n);
        }
        // VFALCO simple algorithm but
        // costs one extra memmove
        erase_all_impl(i0, id);
    }
    insert_impl(
        string_to_field(name),
        name, value, h_.count);
}

//------------------------------------------------

void
fields_base::
set_content_length(
    std::uint64_t n)
{
    detail::number_string s(n);
    set(field::content_length, s.str());
}

//------------------------------------------------
//
// (implementation)
//
//------------------------------------------------

void
fields_base::
clear_impl() noexcept
{
    if(! h_.buf)
        return;
    using H =
        detail::header;
    auto const& h =
        *H::get_default(
            h_.kind);
    h.assign_to(h_);
    std::memcpy(
        h_.buf,
        h.cbuf,
        h_.size);
}

// copy start line and fields
void
fields_base::
copy_impl(
    detail::header const& h)
{
    BOOST_ASSERT(
        h.kind == ph_->kind);
    if(! h.is_default())
    {
        auto n =
            detail::buffer_needed(
                h.size, h.count);
        if(n <= h_.cap)
        {
            // no realloc
            h.assign_to(h_);
            h.copy_table(
                h_.buf + h_.cap);
            std::memcpy(
                h_.buf,
                h.cbuf,
                h.size);
            return;
        }
    }
    fields_base tmp(h);
    tmp.h_.swap(h_);
}

char*
fields_base::
set_prefix_impl(
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
            detail::header::table ft(
                h_.buf + h_.cap);
            h_.copy_table(buf + n0);
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

void
fields_base::
set_content_length_impl(
    std::uint64_t n)
{
    set(field::content_length,
        detail::number_string(n));
}

void
fields_base::
set_chunked_impl(bool value)
{
    if(value)
    {
        // set chunked
        if(h_.te.chunked_count == 0)
        {
            append(
                field::transfer_encoding,
                "chunked");
            return;
        }
    }
    else
    {
        // clear chunked

    }
}

//------------------------------------------------

// return i-th field absolute offset
std::size_t
fields_base::
offset(
    detail::header::table ft,
    std::size_t i) const noexcept
{
    if(i < h_.count)
        return h_.prefix + ft[i].np;
    // make final CRLF the last "field"
    BOOST_ASSERT(i == h_.count);
    return h_.size - 2;
}

// return i-th field absolute length
std::size_t
fields_base::
length(
    detail::header::table ft,
    std::size_t i) const noexcept
{
    return
        offset(ft, i + 1) -
        offset(ft, i);
}

//------------------------------------------------

// erase i, without updating metadata
void
fields_base::
raw_erase(
    std::size_t i) noexcept
{
    BOOST_ASSERT(i < h_.count);
    BOOST_ASSERT(h_.buf != nullptr);
    auto ft = h_.tab();
    auto const p0 = offset(ft, i);
    auto const p1 = offset(ft, i + 1);
    std::memmove(
        h_.buf + p0,
        h_.buf + p1,
        h_.size - p1);
    auto const n = p1 - p0;
    --h_.count;
    for(;i < h_.count; ++i)
        ft[i] = ft[i + 1] - n;
    h_.size = static_cast<
        off_t>(h_.size - n);
}

// erase fields matching i0
// name, without updating metadata.
std::size_t
fields_base::
raw_erase_all(
    std::size_t i0) noexcept
{
    std::size_t n = 1;
    auto const* e = &h_.tab()[i0];
    auto const p = h_.buf + h_.prefix;
    auto const name = string_view(
        p + e->np, e->nn);
    // backwards to reduce memmoves
    std::size_t i = h_.count - 1;
    e = &h_.tab()[i];
    while(i != i0)
    {
        if(bnf::iequals(
            string_view(
                p + e->np, e->nn),
            name))
        {
            raw_erase(i);
            ++n;
        }
        --i;
        ++e;
    }
    raw_erase(i0);
    return n;
}

// insert without updating metadata
void
fields_base::
raw_insert(
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
        // no reallocation
        BOOST_ASSERT(h_.buf != nullptr);
        detail::copied_strings cs(
            string_view(h_.cbuf, h_.size));
        name = cs.maybe_copy(name);
        value = cs.maybe_copy(value);
        detail::header::table ft(
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
        auto const ft0 = h_.tab();
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
        h_.copy_table(buf + n, before);
        detail::header::table ft(buf + n);
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
        // new allocation
        BOOST_ASSERT(h_.buf == nullptr);
        BOOST_ASSERT(before == 0);
        BOOST_ASSERT(h_.count == 0);

        // default string
        BOOST_ASSERT(h_.is_default());
        string_view s(h_.cbuf, h_.size);

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
        auto& e = h_.tab()[before];
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

// unconditionally set i-th field,
// without updating metadata.
void
fields_base::
raw_set(
    std::size_t i,
    string_view value)
{
    auto ft0 = h_.tab();
    auto const nn = ft0[i].nn;
    // total size minus old field
    auto const n0 =
        h_.size - length(ft0, i);
    // size of new field
    auto const n1 = nn + 2 +
        value.size() + 2;
    if(n0 + n1 > max_off_t)
        detail::throw_length_error(
            "too large",
            BOOST_CURRENT_LOCATION);
    auto const n =
        detail::buffer_needed(
            n0 + n1, h_.count);
    if(h_.cap >= n)
    {
        BOOST_ASSERT(h_.buf != nullptr);
        detail::copied_strings cs(
            string_view(h_.cbuf, h_.size));
        value = cs.maybe_copy(value);
        auto const pos =
            offset(ft0, i + 1);
        std::memmove(
            h_.buf + h_.prefix + n1,
            h_.buf + pos,
            h_.size - pos);
        char* dest = h_.buf +
            offset(ft0, i) + nn + 2;
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';
        auto const dn =
            length(ft0, i) - n1;
        // adjust table
        for(std::size_t j = h_.count - 1;
                j > i; --j)
            ft0[j] = ft0[j] - dn;
        // update table entry
        auto& e = ft0[i];
        e.vp = e.np + e.nn + 2;
        e.vn = static_cast<
            off_t>(value.size());
        h_.size = static_cast<
            off_t>(n0 + n1);
        return;
    }

    // grow
    BOOST_ASSERT(h_.cap > 0);
    BOOST_ASSERT(h_.buf != nullptr);
    auto buf = new char[n];
    // copy existing fields
    // and write new field
    auto pos = offset(ft0, i);
    char* dest = buf;
    std::memcpy(
        dest,
        h_.buf,
        pos + nn + 2);
    dest += pos + nn + 2;
    value.copy(dest, value.size());
    dest += value.size();
    *dest++ = '\r';
    *dest++ = '\n';
    pos = offset(ft0, i + 1);
    std::memcpy(
        dest,
        h_.buf + pos,
        h_.size - pos);

    // write table
    h_.copy_table(buf + n, i + 1);
    detail::header::table ft(buf + n);
    auto const dn = n1 -
        length(ft0, i);
    for(auto j = i + 1;
        j < h_.count; ++j)
        ft[j] = ft0[j] + dn;
    auto& e = ft[i];
    e.vp = e.np + e.nn + 2;
    e.vn = static_cast<
        off_t>(value.size());
    h_.size = static_cast<
        off_t>(n0 + n1);

    delete[] h_.buf;
    h_.buf = buf;
    h_.cbuf = buf;
    h_.cap = n;
}

//------------------------------------------------

// erase i and update metadata
void
fields_base::
erase_impl(
    std::size_t i,
    field id) noexcept
{
    raw_erase(i);
    if(id != field::unknown)
        h_.on_erase(id);
}

// erase all fields with id
// and update metadata
std::size_t
fields_base::
erase_all_impl(
    std::size_t i0,
    field id) noexcept
{
    BOOST_ASSERT(
        id != field::unknown);
    std::size_t n = 1;
    std::size_t i = h_.count - 1;
    auto const ft = h_.tab();
    while(i > i0)
    {
        if(ft[i].id == id)
        {
            raw_erase(i);
            ++n;
        }
        // go backwards to
        // reduce memmoves
        --i;
    }
    raw_erase(i0);
    h_.on_erase_all(id);
    return n;
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
    if(id != field::unknown)
    {
        raw_insert(
            id, name, value, before);
        h_.on_insert(id, value);
        return;
    }
    raw_insert(
        id, name, value, before);
}

} // http_proto
} // boost

#endif
