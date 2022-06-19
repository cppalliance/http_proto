//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_HEADER_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_HEADER_IPP

#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/rfc/digits_rule.hpp>
#include <boost/assert.hpp>
#include <utility>

namespace boost {
namespace http_proto {
namespace detail {

header::
header(detail::kind k) noexcept
    : kind(k)
{
}

fields_table
header::
tab() noexcept
{
    BOOST_ASSERT(cap > 0);
    BOOST_ASSERT(buf != nullptr);
    return fields_table(buf + cap);
}

const_fields_table
header::
ctab() const noexcept
{
    BOOST_ASSERT(cap > 0);
    BOOST_ASSERT(cbuf != nullptr);
    return const_fields_table(cbuf + cap);
}

std::size_t
header::
find(
    field id) const noexcept
{
    if(count == 0)
        return 0;
    std::size_t i = 0;
    auto const* p = &ctab()[0];
    while(i < count)
    {
        if(p->id == id)
            break;
        ++i;
        --p;
    }
    return i;
}

std::size_t
header::
find(
    string_view name) const noexcept
{
    if(count == 0)
        return 0;
    std::size_t i = 0;
    auto const* p = &ctab()[0];
    while(i < count)
    {
        string_view s(
            cbuf + prefix + p->np,
            p->nn);
        if(bnf::iequals(s, name))
            break;
        ++i;
        --p;
    }
    return i;
}

string_view
header::
name(std::size_t i) const noexcept
{
    auto const* p = &ctab()[i];
    return string_view(
        cbuf + prefix + p->np, p->nn);
}

void
header::
reset() noexcept
{
    auto buf_ = buf;
    auto cbuf_ = cbuf;
    auto cap_ = cap;

    *this = header(kind);

    buf = buf_;
    cbuf = cbuf_;
    cap = cap_;
}

// called after a field is inserted
void
header::
on_insert(
    field id,
    string_view v) noexcept
{
    switch(id)
    {
    case field::content_length:
        return on_insert_content_length(id, v);
    default:
        break;
    }
}

void
header::
on_insert_content_length(
    field id,
    string_view v) noexcept
{
    (void)id;
    ++cl.count;
    error_code ec;
    digits_rule t;
    grammar::parse_string(v, ec, t);
    if( ! ec.failed() &&
        ! t.overflow)
    {
        if(cl.count == 1)
        {
            // first one
            cl.value = t.v;
            cl.has_value = true;
            return;
        }

        if(cl.has_value)
        {
            if(t.v == cl.value)
            {
                // matching dupe is ok
                return;
            }

            // mismatch
        }
        else
        {
            BOOST_ASSERT(
                cl.value == 0);
            return;
        }
    }

    cl.value = 0;
    cl.has_value = false;
}

//------------------------------------------------

void
parse_start_line(
    header& h,
    std::size_t new_size,
    error_code& ec) noexcept
{
    BOOST_ASSERT(! ec.failed());
    BOOST_ASSERT(h.prefix == 0);
    BOOST_ASSERT(h.size == 0);
    BOOST_ASSERT(new_size > h.size);

    // VFALCO do we need a separate
    //        limit on start line?

    auto const it0 = h.cbuf;
    auto const end = it0 + new_size;
    char const* it = it0;

    // request
    if(h.kind == detail::kind::request)
    {
        request_line_rule t;
        if(grammar::parse(it, end, ec, t))
        {
            h.version = t.v;
            h.req.method = t.m;
            h.req.method_len = static_cast<
                off_t>(t.ms.size());
            h.req.target_len = static_cast<
                off_t>(t.t.size());
            h.prefix = static_cast<
                off_t>(it - it0);
            h.size = h.prefix;
            return;
        }
        if(ec == grammar::error::incomplete)
            return;
        return;
    }

    // response
    BOOST_ASSERT(h.kind ==
        detail::kind::response);
    status_line_rule t;
    if(grammar::parse(it, end, ec, t))
    {
        h.version = t.v;
        h.res.status_int = t.status_int;
        h.res.status =
            int_to_status(t.status_int);
        h.prefix = static_cast<
            off_t>(it - it0);
        h.size = h.prefix;
        return;
    }
    if(ec == grammar::error::incomplete)
        return;
}

// returns: true if we added a field
bool
parse_field(
    header& h,
    std::size_t new_size,
    field& id,
    string_view& v,
    error_code& ec) noexcept
{
    auto const it0 = h.cbuf + h.size;
    auto const end = h.cbuf + new_size;
    char const* it = it0;
    field_rule t;
    grammar::parse(it, end , ec, t);
    if(ec.failed())
    {
        if(ec == grammar::error::end)
        {
            // final CRLF
            ec.clear();
            h.size = static_cast<off_t>(
                it - h.cbuf);
        }
        return false;
    }
    if(t.v.has_obs_fold)
    {
        // obs fold not allowed in test views
        BOOST_ASSERT(h.buf != nullptr);
        remove_obs_fold(h.buf + h.size, it);
    }
    v = t.v.value;
    id = string_to_field(t.v.name);
    h.size = static_cast<off_t>(
        it - h.cbuf);

    // add field table entry
    if(h.buf != nullptr)
    {
        auto& e = fields_table(
            h.buf + h.cap)[h.count];
        auto const base =
            h.buf + h.prefix;
        e.np = static_cast<off_t>(
            t.v.name.data() - base);
        e.nn = static_cast<off_t>(
            t.v.name.size());
        e.vp = static_cast<off_t>(
            t.v.value.data() - base);
        e.vn = static_cast<off_t>(
            t.v.value.size());
        e.id = id;

    #if 0
        // VFALCO handling zero-length value?
        if(fi.value_len > 0)
            fi.value_pos = static_cast<
                off_t>(t.v.value.data() - h_.buf);
        else
            fi.value_pos = 0; // empty string
    #endif
    }
    ++h.count;
    h.on_insert(id, v);
    return true;
}

} // detail
} // http_proto
} // boost

#endif
