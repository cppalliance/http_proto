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
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/http_proto/rfc/list_rule.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>
#include <boost/http_proto/rfc/transfer_encoding_rule.hpp>
#include <boost/http_proto/rfc/upgrade_rule.hpp>
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
    : kind(detail::kind::response)
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
header(empty v) noexcept
    : kind(v.param)
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

// called when the start-line changes
void
header::
on_start_line()
{
    if(kind ==
        detail::kind::response)
    {
        // maybe status_int
        update_payload();
    }
}

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
        return on_insert_te();
    case field::upgrade:
        return on_insert_up(v);
    }
}

// called when Content-Length is inserted
void
header::
on_insert_clen(string_view v)
{
    static
    constexpr
    grammar::unsigned_rule<
        std::uint64_t> num_rule{};

    ++md.content_length.count;
    if(md.content_length.ec.failed())
        return;
    auto rv =
        grammar::parse(v, num_rule);
    if(! rv)
    {
        // parse failure
        BOOST_HTTP_PROTO_SET_EC(
            md.content_length.ec,
            error::bad_content_length);
        md.content_length.value = 0;
        update_payload();
        return;
    }
    if(md.content_length.count == 1)
    {
        // one value
        md.content_length.ec = {};
        md.content_length.value = *rv;
        update_payload();
        return;
    }
    if(*rv == md.content_length.value)
    {
        // ok: duplicate value
        return;
    }
    // bad: different values
    BOOST_HTTP_PROTO_SET_EC(
        md.content_length.ec,
        error::multiple_content_length);
    md.content_length.value = 0;
    update_payload();
}

// called when Connection is inserted
void
header::
on_insert_con(string_view v)
{
    ++md.connection.count;
    if(md.connection.ec.failed())
        return;
    auto rv = grammar::parse(
        v, list_rule(token_rule, 1));
    if(! rv)
    {
        BOOST_HTTP_PROTO_SET_EC(
            md.connection.ec,
            error::bad_connection);
        return;
    }
    md.connection.ec = {};
    for(auto const& t : *rv)
    {
        if(grammar::ci_is_equal(
                t, "close"))
            md.connection.close = true;
        else if(grammar::ci_is_equal(
                t, "keep-alive"))
            md.connection.keep_alive = true;
        else if(grammar::ci_is_equal(
                t, "upgrade"))
            md.connection.upgrade = true;
    }
}

// called when Transfer-Encoding is inserted
void
header::
on_insert_te()
{
    ++md.transfer_encoding.count;
    if(md.transfer_encoding.ec.failed())
        return;
    auto const n =
        md.transfer_encoding.count;
    md.transfer_encoding = {};
    md.transfer_encoding.count = n;
    for(auto const& s :
        fields_view_base::subrange(
            this, find(field::transfer_encoding)))
    {
        auto rv = grammar::parse(
            s, transfer_encoding_rule);
        if(! rv)
        {
            // parse error
            BOOST_HTTP_PROTO_SET_EC(
                md.transfer_encoding.ec,
                error::bad_transfer_encoding);
            md.transfer_encoding.codings = 0;
            md.transfer_encoding.is_chunked = false;
            update_payload();
            return;
        }
        md.transfer_encoding.codings += rv->size();
        for(auto const& t : *rv)
        {
            if(! md.transfer_encoding.is_chunked)
            {
                if(t.id == transfer_coding::chunked)
                    md.transfer_encoding.is_chunked = true;
                continue;
            }
            if(t.id == transfer_coding::chunked)
            {
                // chunked appears twice
                BOOST_HTTP_PROTO_SET_EC(
                    md.transfer_encoding.ec,
                    error::bad_transfer_encoding);
                md.transfer_encoding.codings = 0;
                md.transfer_encoding.is_chunked = false;
                update_payload();
                return;
            }
            // chunked must be last
            BOOST_HTTP_PROTO_SET_EC(
                md.transfer_encoding.ec,
                error::bad_transfer_encoding);
            md.transfer_encoding.codings = 0;
            md.transfer_encoding.is_chunked = false;
            update_payload();
            return;
        }
    }
    update_payload();
}

// called when Upgrade is inserted
void
header::
on_insert_up(string_view v)
{
    ++md.upgrade.count;
    if(md.upgrade.ec.failed())
        return;
    if( version !=
        http_proto::version::http_1_1)
    {
        BOOST_HTTP_PROTO_SET_EC(
            md.upgrade.ec,
            error::bad_upgrade);
        md.upgrade.websocket = false;
        return;
    }
    auto rv = grammar::parse(
        v, upgrade_rule);
    if(! rv)
    {
        BOOST_HTTP_PROTO_SET_EC(
            md.upgrade.ec,
            error::bad_upgrade);
        md.upgrade.websocket = false;
        return;
    }
    if(! md.upgrade.websocket)
    {
        for(auto const& t : *rv)
        {
            if( grammar::ci_is_equal(
                    t.name, "websocket") &&
                t.version.empty())
            {
                md.upgrade.websocket = true;
                break;
            }
        }
    }
}

//------------------------------------------------

// called when one field is erased
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
    }
}

// called when Content-Length is erased
void
header::
on_erase_clen()
{
    BOOST_ASSERT(
        md.content_length.count > 0);
    --md.content_length.count;
    if(md.content_length.count == 0)
    {
        // no Content-Length
        md.content_length = {};
        update_payload();
        return;
    }
    if(! md.content_length.ec.failed())
    {
        // removing a duplicate value
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

// called when Connection is erased
void
header::
on_erase_con()
{
    BOOST_ASSERT(
        md.connection.count > 0);
    // reset and re-insert
    auto n = md.connection.count - 1;
    auto const p = cbuf + prefix;
    auto const* e = &tab()[0];
    md.connection = {};
    while(n > 0)
    {
        if(e->id == field::connection)
            on_insert_con(string_view(
                p + e->vp, e->vn));
        --n;
        --e;
    }
}

// called when Transfer-Encoding is erased
void
header::
on_erase_te()
{
    BOOST_ASSERT(
        md.transfer_encoding.count > 0);
    --md.transfer_encoding.count;
    if(md.transfer_encoding.count == 0)
    {
        // no more Transfer-Encoding
        md.transfer_encoding = {};
        update_payload();
        return;
    }
    // re-insert everything
    --md.transfer_encoding.count;
    on_insert_te();
}

// called when Upgrade is erased
void
header::
on_erase_up()
{
    BOOST_ASSERT(
        md.upgrade.count > 0);
    --md.upgrade.count;
    if(md.upgrade.count == 0)
    {
        // no Upgrade
        md.upgrade = {};
        return;
    }
    // reset and re-insert
    auto n = md.upgrade.count;
    auto const p = cbuf + prefix;
    auto const* e = &tab()[0];
    md.upgrade = {};
    while(n > 0)
    {
        if(e->id == field::upgrade)
            on_insert_up(string_view(
                p + e->vp, e->vn));
        --n;
        --e;
    }
}

// called when all fields with id are removed
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
        md.connection = {};
        return;

    case field::content_length:
        md.content_length = {};
        update_payload();
        return;

    case field::transfer_encoding:
        md.transfer_encoding = {};
        update_payload();
        return;

    case field::upgrade:
        md.upgrade = {};
        return;
    }
}

//------------------------------------------------

/*  References:

    3.3.  Message Body
    https://datatracker.ietf.org/doc/html/rfc7230#section-3.3

    3.3.1.  Transfer-Encoding
    https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.1

    3.3.2.  Content-Length
    https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.2
*/
void
header::
update_payload() noexcept
{
    BOOST_ASSERT(kind !=
        detail::kind::fields);
    if(md.manual_payload)
    {
        // e.g. response to
        // a HEAD request
        return;
    }

/*  If there is an error in either Content-Length
    or Transfer-Encoding, then the payload is
    undefined. Clients should probably close the
    connection. Servers can send a Bad Request
    and avoid reading any payload bytes.
*/
    if(md.content_length.ec.failed())
    {
        // invalid Content-Length
        md.payload = payload::error;
        md.payload_size = 0;
        return;
    }
    if(md.transfer_encoding.ec.failed())
    {
        // invalid Transfer-Encoding
        md.payload = payload::error;
        md.payload_size = 0;
        return;
    }

/*  A sender MUST NOT send a Content-Length
    header field in any message that contains
    a Transfer-Encoding header field.
    https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.2
*/
    if( md.content_length.count > 0 &&
        md.transfer_encoding.count > 0)
    {
        md.payload = payload::error;
        md.payload_size = 0;
        return;
    }

    if(kind == detail::kind::response)
        goto do_response;

    //--------------------------------------------

/*  The presence of a message body in a
    request is signaled by a Content-Length
    or Transfer-Encoding header field. Request
    message framing is independent of method
    semantics, even if the method does not
    define any use for a message body.
*/
    if(md.content_length.count > 0)
    {
        if(md.content_length.value > 0)
        {
            // non-zero Content-Length
            md.payload = payload::size;
            md.payload_size = md.content_length.value;
            return;
        }
        // Content-Length: 0
        md.payload = payload::none;
        md.payload_size = 0;
        return;
    }
    if(md.transfer_encoding.is_chunked)
    {
        // chunked
        md.payload = payload::chunked;
        md.payload_size = 0;
        return;
    }
    // no payload
    md.payload = payload::none;
    md.payload_size = 0;
    return;

    //--------------------------------------------
do_response:

    if( res.status_int /  100 == 1 ||   // 1xx e.g. Continue
        res.status_int == 204 ||        // No Content
        res.status_int == 304)          // Not Modified
    {
    /*  The correctness of any Content-Length
        here is defined by the particular
        resource, and cannot be determined
        here. In any case there is no payload.
    */
        md.payload = payload::none;
        md.payload_size = 0;
        return;
    }
    if(md.content_length.count > 0)
    {
        if(md.content_length.value > 0)
        {
            // Content-Length > 0
            md.payload = payload::size;
            md.payload_size = md.content_length.value;
            return;
        }
        // Content-Length: 0
        md.payload = payload::none;
        md.payload_size = 0;
        return;
    }
    if(md.transfer_encoding.is_chunked)
    {
        // chunked
        md.payload = payload::chunked;
        md.payload_size = 0;
        return;
    }

    // eof needed
    md.payload = payload::to_eof;
    md.payload_size = 0;
}

/*  References:

    6.3.  Persistence
    https://datatracker.ietf.org/doc/html/rfc7230#section-6.3
*/
bool
header::
keep_alive() const noexcept
{
    if(md.payload == payload::error)
        return false;
    if( version ==
        http_proto::version::http_1_1)
    {
        if(md.connection.close)
            return false;
    }
    else
    {
        if(! md.connection.keep_alive)
            return false;
    }
    // can't use to_eof in requests
    BOOST_ASSERT(
        kind != detail::kind::request ||
        md.payload != payload::to_eof);
    if(md.payload == payload::to_eof)
        return false;
    return true;
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
