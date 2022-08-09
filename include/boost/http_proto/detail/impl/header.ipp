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
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/rfc/detail/rules.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/unsigned_rule.hpp>
#include <boost/assert.hpp>
#include <utility>

namespace boost {
namespace http_proto {
namespace detail {

//------------------------------------------------

constexpr
header::
header(fields_tag) noexcept
    : kind(detail::kind::fields)
    , cbuf("\r\n")
    , size(2)
    , fld{}
{
}

constexpr
header::
header(request_tag) noexcept
    : kind(detail::kind::request)
    , cbuf("GET / HTTP/1.1\r\n\r\n")
    , size(18)
    , prefix(16)
    , req{ 3, 1,
        http_proto::method::get }
{
}

constexpr
header::
header(response_tag) noexcept
    : kind(detail::kind::request)
    , cbuf("HTTP/1.1 200 OK\r\n\r\n")
    , size(19)
    , prefix(17)
    , res{ 200,
        http_proto::status::ok }
{
}

header::
header(detail::kind k) noexcept
    : header(*get_default(k))
{
}

//------------------------------------------------

inline
auto
header::
entry::
operator+(
    std::size_t dv) const noexcept ->
        entry
{
    return {
        static_cast<
            off_t>(np + dv),
        nn,
        static_cast<
            off_t>(vp + dv),
        vn,
        id };
}

inline
auto
header::
entry::
operator-(
    std::size_t dv) const noexcept ->
        entry
{
    return {
        static_cast<
            off_t>(np - dv),
        nn,
        static_cast<
            off_t>(vp - dv),
        vn,
        id };
}

//------------------------------------------------

// return total bytes needed
// to store message of `size`
// bytes and `count` fields.
inline
std::size_t
buffer_needed(
    std::size_t size,
    std::size_t count) noexcept
{
    // make sure `size` is big enough
    // to hold the largest default buffer:
    // "HTTP/1.1 200 OK\r\n\r\n"
    if( size < 19)
        size = 19;
    static constexpr auto A =
        alignof(header::entry);
    return A * (
        (size + A - 1) / A) +
            (count * sizeof(
                header::entry));
}

//------------------------------------------------


header const*
header::
get_default(detail::kind k) noexcept
{
    static header h[3] = {
        fields_tag{},
        request_tag{},
        response_tag{}};
    return &h[k];
}

auto
header::
tab() const noexcept ->
    table
{
    BOOST_ASSERT(cap > 0);
    BOOST_ASSERT(buf != nullptr);
    return table(buf + cap);
}

// return true if s is a default string
bool
header::
is_default() const noexcept
{
    return
        (cbuf == get_default(
            detail::kind::fields)->cbuf) ||
        (cbuf == get_default(
            detail::kind::request)->cbuf) ||
        (cbuf == get_default(
            detail::kind::response)->cbuf);
}

std::size_t
header::
find(
    field id) const noexcept
{
    if(count == 0)
        return 0;
    std::size_t i = 0;
    auto const* p = &tab()[0];
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
    auto const* p = &tab()[0];
    while(i < count)
    {
        string_view s(
            cbuf + prefix + p->np,
            p->nn);
        if(grammar::ci_is_equal(s, name))
            break;
        ++i;
        --p;
    }
    return i;
}

void
header::
copy_table(
    void* dest,
    std::size_t n) const noexcept
{
    std::memcpy(
        reinterpret_cast<
            entry*>(dest) - n,
        reinterpret_cast<
            entry const*>(
                cbuf + cap) - n,
        n * sizeof(entry));
}

void
header::
copy_table(
    void* dest) const noexcept
{
    copy_table(dest, count);
}

// assign all the members but
// preserve the allocated memory
void
header::
assign_to(
    header& dest) const noexcept
{
    auto const buf_ = dest.buf;
    auto const cbuf_ = dest.cbuf;
    auto const cap_ = dest.cap;
    dest = *this;
    dest.buf = buf_;
    dest.cbuf = cbuf_;
    dest.cap = cap_;
}

void
header::
swap(header& h) noexcept
{
    std::swap(cbuf, h.cbuf);
    std::swap(buf, h.buf);
    std::swap(cap, h.cap);
    std::swap(size, h.size);
    std::swap(count, h.count);
    std::swap(prefix, h.prefix);
    std::swap(version, h.version);
    switch(kind)
    {
    case detail::kind::fields:
        break;
    case detail::kind::request:
        std::swap(
            req.method_len, h.req.method_len);
        std::swap(
            req.target_len, h.req.target_len);
        std::swap(req.method, h.req.method);
        break;
    case detail::kind::response:
        std::swap(
            res.status_int, h.res.status_int);
        std::swap(res.status, h.res.status);
        break;
    }
}

//------------------------------------------------
//
// Metadata
//
//------------------------------------------------
// update metadata for
// one field id erased
void
header::
on_erase(field id) noexcept
{
    if(kind == detail::kind::fields)
        return;
    switch(id)
    {
    case field::connection:
    {
        //auto rv = grammar::parse(
        break;
    }
    case field::content_length:
    {
        BOOST_ASSERT(clen.count > 0);
        if(clen.count == 1)
        {
            // no more Content-Length
            clen = {};
            return;
        }
        --clen.count;
        if(clen.has_value)
        {
            // don't scan, remaining
            // Content-Length are same
            return;
        }
        // scan all Content-Length
        auto const p = cbuf + prefix;
        auto const* e = &tab()[0];
        std::size_t n = clen.count;
        clen = {};
        while(n > 0)
        {
            if(e->id == id)
                on_insert(id,
                    string_view(
                        p + e->vp,
                        e->vn));
            --n;
            --e;
        }
        break;
    }

    default:
        break;
    }
}

// update metadata for
// all field id erased
void
header::
on_erase_all(
    field id) noexcept
{
    if(kind == detail::kind::fields)
        return;
    switch(id)
    {
    case field::connection:
        con = {};
        return;

    case field::content_length:
        clen = {};
        return;

    default:
        break;
    }
}

//------------------------------------------------

// called after a field is inserted
void
header::
on_insert(
    field id,
    string_view v) noexcept
{
    if(kind == detail::kind::fields)
        return;
    switch(id)
    {
    case field::content_length:
    {
        ++clen.count;
        auto rv = grammar::parse(v,
            grammar::unsigned_rule<
                std::uint64_t>{});
        if(rv.has_value())
        {
            if(clen.count == 1)
            {
                // first one
                clen.value = *rv;
                clen.has_value = true;
                return;
            }

            if(clen.has_value)
            {
                if(*rv == clen.value)
                {
                    // matching dupe is ok
                    return;
                }

                // mismatch
            }
            else
            {
                BOOST_ASSERT(
                    clen.value == 0);
                return;
            }
        }

        // overflow or parse error
        clen.value = 0;
        clen.has_value = false;
        return;
    }

    case field::transfer_encoding:
    {
        return;
    }

    default:
        break;
    }
}


void
header::
update_payload() noexcept
{
#if 0
    BOOST_ASSERT(kind !=
        detail::kind::fields);

    if(kind == detail::kind::request)
    {
    /*  https://datatracker.ietf.org/doc/html/rfc7230#section-3.3

        The presence of a message body in a
        request is signaled by a Content-Length or
        Transfer-Encoding header field. Request message
        framing is independent of method semantics, even
        if the method does not define any use for a
        message body.
    */
        if(clen.count > 0)
        {
            if(body_limit_.has_value() &&
               len_ > body_limit_)
            {
                ec = error::body_limit;
                return;
            }
            if(len_ > 0)
            {
                f_ |= flagHasBody;
                state_ = state::body0;
            }
            else
            {
                state_ = state::complete;
            }
        }
        else if(f_ & flagChunked)
        {
            f_ |= flagHasBody;
            state_ = state::chunk_header0;
        }
        else
        {
            len_ = 0;
            len0_ = 0;
            state_ = state::complete;
        }
    }
    else
    {
        BOOST_ASSERT(kind ==
            detail::kind::response);

        // RFC 7230 section 3.3
        // https://tools.ietf.org/html/rfc7230#section-3.3

        if( //(f_ & flagSkipBody) ||          // e.g. response to a HEAD request
            res.status_int /  100 == 1 ||   // 1xx e.g. Continue
            res.status_int == 204 ||        // No Content
            res.status_int == 304)          // Not Modified
        {
            // message semantics indicate no payload
            // present regardless of header contents
            //state_ = state::complete;
        }
        else if(f_ & flagContentLength)
        {
            if(len_ > 0)
            {
                f_ |= flagHasBody;
                state_ = state::body0;

                if(body_limit_.has_value() &&
                   len_ > body_limit_)
                {
                    ec = error::body_limit;
                    return;
                }
            }
            else
            {
                state_ = state::complete;
            }
        }
        else if(f_ & flagChunked)
        {
            //f_ |= flagHasBody;
            pay.kind = payload::chunked;
        }
        else
        {
            //f_ |= flagHasBody;
            //f_ |= flagNeedEOF;
            pay.kind = payload::to_eof;
        }
    }
#endif
}

//------------------------------------------------

static
void
parse_request_line(
    header& h,
    std::size_t new_size,
    error_code& ec) noexcept
{
    auto const it0 = h.cbuf;
    auto const end = it0 + new_size;
    char const* it = it0;

    auto rv = grammar::parse(
        it, end, request_line_rule);
    if(! rv)
    {
        ec = rv.error();
        return;
    }
    switch(std::get<2>(*rv))
    {
    case 10:
        h.version = version::http_1_0;
        break;
    case 11:
        h.version = version::http_1_1;
        break;
    default:
        ec = error::bad_version;
        return;
    }
    auto sm = std::get<0>(*rv);
    auto st = std::get<1>(*rv);
    h.req.method = string_to_method(sm);
    h.req.method_len = static_cast<
        off_t>(sm.size());
    h.req.target_len = static_cast<
        off_t>(st.size());
    h.prefix = static_cast<
        off_t>(it - it0);
    h.size = h.prefix;
    h.update_payload();
    ec = {};
}

static
void
parse_status_line(
    header& h,
    std::size_t new_size,
    error_code& ec) noexcept
{
    auto const it0 = h.cbuf;
    auto const end = it0 + new_size;
    char const* it = it0;

    auto rv = grammar::parse(
        it, end, status_line_rule);
    if(! rv)
    {
        ec = rv.error();
        return;
    }
    switch(std::get<0>(*rv))
    {
    case 10:
        h.version = version::http_1_0;
        break;
    case 11:
        h.version = version::http_1_1;
        break;
    default:
        ec = error::bad_version;
        return;
    }
    h.res.status_int = static_cast<
        unsigned short>(std::get<1>(*rv).v);
    h.res.status = std::get<1>(*rv).st;
    h.prefix = static_cast<
        off_t>(it - it0);
    h.size = h.prefix;
    h.update_payload();
    ec = {};
}

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

    if(h.kind == detail::kind::request)
        return parse_request_line(
            h, new_size, ec);
    parse_status_line(h, new_size, ec);
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
    auto rv = grammar::parse(
        it, end, field_rule);
    if(rv.has_error())
    {
        ec = rv.error();
        if(ec == grammar::error::range_end)
        {
            // final CRLF
            ec.clear();
            h.size = static_cast<off_t>(
                it - h.cbuf);
        }
        return false;
    }
    if(rv->has_obs_fold)
    {
        // obs fold not allowed in test views
        BOOST_ASSERT(h.buf != nullptr);
        remove_obs_fold(h.buf + h.size, it);
    }
    v = rv->value;
    id = string_to_field(rv->name);
    h.size = static_cast<off_t>(
        it - h.cbuf);

    // add field table entry
    if(h.buf != nullptr)
    {
        auto& e = header::table(
            h.buf + h.cap)[h.count];
        auto const base =
            h.buf + h.prefix;
        e.np = static_cast<off_t>(
            rv->name.data() - base);
        e.nn = static_cast<off_t>(
            rv->name.size());
        e.vp = static_cast<off_t>(
            rv->value.data() - base);
        e.vn = static_cast<off_t>(
            rv->value.size());
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
