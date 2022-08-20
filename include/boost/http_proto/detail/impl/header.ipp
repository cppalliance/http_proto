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
#include <boost/http_proto/detail/make_list.hpp>
#include <boost/http_proto/rfc/list_rule.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>
#include <boost/http_proto/rfc/transfer_encoding_rule.hpp>
#include <boost/http_proto/rfc/detail/rules.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/range_rule.hpp>
#include <boost/url/grammar/recycled.hpp>
#include <boost/url/grammar/unsigned_rule.hpp>
#include <boost/assert.hpp>
#include <string>
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

header const*
header::
get_default(detail::kind k) noexcept
{
    static constexpr header h[3] = {
        fields_tag{},
        request_tag{},
        response_tag{}};
    return &h[k];
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
    std::swap(pay, h.pay);
    std::swap(con, h.con);
    std::swap(te, h.te);
    std::swap(up, h.up);
    std::swap(md, h.md);
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

// called after a field is inserted
void
header::
on_insert(field id, string_view v)
{
    if(kind == detail::kind::fields)
        return;
    switch(id)
    {
    case field::connection:
        return on_insert_con(v);
    case field::content_length:
        return on_insert_clen(v);
    case field::transfer_encoding:
        return on_insert_te(v);
    case field::upgrade:
        return on_insert_up(v);
    default:
        break;
    }
}

void
header::
on_insert_clen(string_view v)
{
/*
    Content-Length = 1*DIGIT
*/
    ++md.content_length.count;
    //if(md.content_length.ec.failed())
    if(md.content_length.ec != error::success)
        return;
    auto rv = grammar::parse(v,
        grammar::unsigned_rule<
            std::uint64_t>{});
    if(! rv)
    {
        // parse failure
        //md.content_length.ec = rv.error();
        md.content_length.ec = error::bad_content_length;
        md.content_length.has_value = 0;
        update_payload();
        return;
    }
    if(md.content_length.count == 1)
    {
        md.content_length.value = *rv;
        md.content_length.has_value = true;
        update_payload();
        return;
    }
    BOOST_ASSERT(md.content_length.has_value);
    if(*rv == md.content_length.value)
    {
        // duplicate
        return;
    }
    md.content_length.value = 0;
    md.content_length.has_value = false;
    md.content_length.ec = error::multiple_content_length;
}

void
header::
on_insert_con(string_view v)
{
/*
    Connection        = 1#connection-option
    connection-option = token
*/
    ++con.count;
    auto rv = grammar::parse(
        v, list_rule(token_rule, 1));
    if(! rv)
    {
        con.error = true;
        return;
    }
    for(auto t : *rv)
    {
        if(grammar::ci_is_equal(
                t, "close"))
            ++con.n_close;
        else if(grammar::ci_is_equal(
                t, "keep-alive"))
            ++con.n_keepalive;
        else if(grammar::ci_is_equal(
                t, "upgrade"))
            ++con.n_upgrade;
    }
}

void
header::
on_insert_te(string_view v)
{
/*
    Transfer-Encoding  = 1#transfer-coding
    transfer-coding    = "chunked"
                        / "compress"
                        / "deflate"
                        / "gzip"
                        / transfer-extension
    transfer-extension = token *( OWS ";" OWS transfer-parameter )
    transfer-parameter = token BWS "=" BWS ( token / quoted-string )
*/
    auto n = te.count + 1;
    te = {};
    te.count = n;
    grammar::recycled_ptr<
        std::string> rs(nullptr);
    if(n > 1)
    {
        rs.acquire();
        rs->clear();
        detail::make_list(*rs,
            field::transfer_encoding,
                n, *this);
        v = *rs;
    }
    auto rv = grammar::parse(
        v, transfer_encoding_rule);
    if(rv)
    {
        auto const& t = *rv;
        te.codings = t.size();
        auto next = t.begin();
        auto const end = t.end();
        while(next != end)
        {
            auto it = next++;
            if((*it).id ==
                transfer_coding::chunked)
            {
                if(next == end)
                {
                    te.is_chunked = true;
                }
                else
                {
                    // chunked not last!
                    te.error = true;
                    break;
                }
            }
        }
    }
    else
    {
        // parse error
        te.error = true;
    }
    update_payload();
}

void
header::
on_insert_up(string_view v)
{
/*
     Upgrade          = 1#protocol

     protocol         = protocol-name ["/" protocol-version]
     protocol-name    = token
     protocol-version = token
*/
    (void)v;
}

//------------------------------------------------

// update metadata for
// one field id erased
void
header::
on_erase(field id)
{
    if(kind == detail::kind::fields)
        return;
    switch(id)
    {
    case field::connection:
        return on_erase_con();
    case field::content_length:
        return on_erase_clen();
    case field::transfer_encoding:
        return on_erase_te();
    case field::upgrade:
        return on_erase_up();
    default:
        break;
    }
}

void
header::
on_erase_clen()
{
    BOOST_ASSERT(md.content_length.count > 0);
    if(md.content_length.count == 1)
    {
        // no Content-Length
        md.content_length = {};
        update_payload();
        return;
    }
    --md.content_length.count;
    if(md.content_length.has_value)
    {
        // other Content-Length
        // fields have the same value
        //BOOST_ASSERT(! md.content_length.ec.failed());
        BOOST_ASSERT(md.content_length.ec == error::success);
        return;
    }
    // reset and re-insert
    auto n = md.content_length.count;
    auto const p = cbuf + prefix;
    auto const* e = &tab()[0];
    md.content_length = {};
    while(n > 0)
    {
        if(e->id == field::content_length)
            on_insert_clen(string_view(
                p + e->vp, e->vn));
        --n;
        --e;
    }
    update_payload();
}

void
header::
on_erase_con()
{
}

void
header::
on_erase_te()
{
}

void
header::
on_erase_up()
{
}

// update metadata for
// all field id erased
void
header::
on_erase_all(
    field id)
{
    if(kind == detail::kind::fields)
        return;
    switch(id)
    {
    case field::connection:
        con = {};
        return;

    case field::content_length:
        md.content_length = {};
        update_payload();
        return;

    case field::transfer_encoding:
        te = {};
        update_payload();
        return;

    case field::upgrade:
        up = {};
        return;

    default:
        break;
    }
}

//------------------------------------------------

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
        if(md.content_length.count > 0)
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
result<std::size_t>
parse_request_line(
    header& h,
    string_view s) noexcept
{
    auto const it0 = s.data();
    auto const end = it0 + s.size();
    char const* it = it0;
    auto rv = grammar::parse(
        it, end, request_line_rule);
    if(! rv)
        return rv.error();
    // method
    auto sm = std::get<0>(*rv);
    h.req.method = string_to_method(sm);
    h.req.method_len =
        static_cast<off_t>(sm.size());
    // target
    auto st = std::get<1>(*rv);
    h.req.target_len =
        static_cast<off_t>(st.size());
    // version
    switch(std::get<2>(*rv))
    {
    case 10:
        h.version = version::http_1_0;
        break;
    case 11:
        h.version = version::http_1_1;
        break;
    default:
        return error::bad_version;
    }

    h.cbuf = s.data();
    h.prefix =
        static_cast<off_t>(it - it0);
    h.size = h.prefix;
    h.update_payload();
    return h.prefix;
}

static
result<std::size_t>
parse_status_line(
    header& h,
    string_view s) noexcept
{
    auto const it0 = s.data();
    auto const end = it0 + s.size();
    char const* it = it0;
    auto rv = grammar::parse(
        it, end, status_line_rule);
    if(! rv)
        return rv.error();
    // version
    switch(std::get<0>(*rv))
    {
    case 10:
        h.version = version::http_1_0;
        break;
    case 11:
        h.version = version::http_1_1;
        break;
    default:
        return error::bad_version;
    }
    // status-code
    h.res.status_int =
        static_cast<unsigned short>(
            std::get<1>(*rv).v);
    h.res.status = std::get<1>(*rv).st;

    h.cbuf = s.data();
    h.prefix =
        static_cast<off_t>(it - it0);
    h.size = h.prefix;
    h.update_payload();
    return h.prefix;
}

result<std::size_t>
parse_start_line(
    header& h,
    string_view s) noexcept
{
    BOOST_ASSERT(! s.empty());
    BOOST_ASSERT(h.size == 0);
    BOOST_ASSERT(h.prefix == 0);

    // VFALCO do we need a separate
    //        limit on start line?

    if(h.kind == detail::kind::request)
        return parse_request_line(h, s);
    return parse_status_line(h, s);
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
