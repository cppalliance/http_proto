//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_PARSER_IPP

#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/http_proto/bnf/chunk_part.hpp>
#include <boost/http_proto/bnf/connection.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/list.hpp>
#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/transfer_encoding.hpp>
#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/assert.hpp>
#include <boost/none.hpp>
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------

parser::
parser(
    detail::kind k,
    config const& cfg,
    std::size_t buffer_bytes)
    : cfg_(cfg)
    , state_(state::start_line)
    , got_eof_(false)
    , m_{}
{
    // buffer must be large enough to
    // hold a complete header.
    if( buffer_bytes <
        cfg.max_header_size)
        buffer_bytes =
            cfg.max_header_size;

    h_.kind = k;
    h_.cap = buffer_bytes;
    h_.buf = new char[buffer_bytes];
    h_.cbuf = h_.buf;
}

//------------------------------------------------
//
// Special Members
//
//------------------------------------------------

parser::
~parser()
{
    delete[] h_.buf;
}

//------------------------------------------------
//
// Observers
//
//------------------------------------------------

string_view
parser::
body() const noexcept
{
    // VFALCO How about some
    // asserts or exceptions?
    if(! m_.got_chunked)
        return string_view(
            h_.buf + h_.size,
            m_.n_payload);
    return string_view(
        h_.buf +
            h_.size +
            m_.n_chunk,
        m_.n_payload);
}

//------------------------------------------------
//
// Input
//
//------------------------------------------------

void
parser::
clear() noexcept
{
}

void
parser::
reset()
{
    if(got_eof_)
    {
        // new connection, throw
        // out all previous state.
        used_ = 0;
        size_ = 0;
        got_eof_ = false;
    }
    else
    {
        // Throwing out partial data
        // will desync the HTTP stream
        //BOOST_ASSERT(is_complete());
        if( size_ > used_ &&
            used_ > 0)
        {
            // move unused octets to front
            std::memcpy(
                h_.buf,
                h_.buf + used_,
                size_ - used_);
            size_ -= used_;
            used_ = 0;
        }
    }

    m_ = message{};
    state_ = state::start_line;
}

mutable_buffers
parser::
prepare()
{
    if(got_eof_)
    {
        // call reset() after
        // reaching end of stream
        detail::throw_invalid_argument(
            "eof", BOOST_CURRENT_LOCATION);
    }
    mb_ = {
        h_.buf + size_,
        h_.cap - size_ };
    return { &mb_, 1 };
}

void
parser::
commit(
    std::size_t n)
{
    if(got_eof_)
    {
        // call reset() after
        // reaching end of stream
        detail::throw_invalid_argument(
            "eof", BOOST_CURRENT_LOCATION);
    }
    BOOST_ASSERT(n <= h_.cap - size_);
    size_ += n;
}

void
parser::
commit_eof()
{
    got_eof_ = true;
}

//------------------------------------------------
//
// Parsing
//
//------------------------------------------------

void
parser::
parse(
    error_code& ec)
{
    switch(state_)
    {
    case state::start_line:
    {
        BOOST_ASSERT(used_ == 0);
        BOOST_ASSERT(h_.prefix == 0);
        BOOST_ASSERT(h_.size == 0);
        std::size_t n;
        bool const limit = size_ >
            cfg_.max_header_size;
        if(! limit)
            n = size_;
        else
            n = cfg_.max_header_size;
        ec = {};
        auto it = parse_start_line(
            h_.buf, h_.buf + n, ec);
        used_ += it - h_.buf;
        if(ec == grammar::error::incomplete)
        {
            if(! limit)
                return;
            ec = error::header_limit;
            return;
        }
        if(ec.failed())
            return;
        h_.prefix = static_cast<
            off_t>(it - h_.buf);
        state_ = state::header_fields;
        BOOST_FALLTHROUGH;
    }
    case state::header_fields:
    {
        std::size_t n;
        bool const limit = size_ >
            cfg_.max_header_size;
        if(! limit)
            n = size_;
        else
            n = cfg_.max_header_size;
        BOOST_ASSERT(h_.size <
            cfg_.max_header_size);
        ec = {};
        auto start = h_.buf + h_.prefix;
        auto it = parse_fields(
            start, h_.buf + n, ec);
        used_ += it - start;
        if(ec == grammar::error::incomplete)
        {
            if(! limit)
                return;
            ec = error::header_limit;
            return;
        }
        if(ec.failed())
            return;
        h_.size = static_cast<
            off_t>(it - h_.buf);
        break;
    }
    case state::body:
    {
        ec = {};
        return;
    }
    case state::complete:
    {
        ec = error::end_of_message;
        if(! got_eof_)
            return;
        state_ = state::end_of_stream;
        return;
    }
    case state::end_of_stream:
    {
        ec = error::end_of_stream;
        return;
    }
    }

    finish_header(ec);
    if(ec)
        return;
    if(! m_.skip_body)
    {
        state_ = state::body;
    }
    else
    {
        state_ = state::complete;
    }
}

char*
parser::
parse_fields(
    char* const start,
    char const* const end,
    error_code& ec)
{
    field_rule t;
    char* it0 = start;
    char const* it = start;
    for(;;)
    {
        grammar::parse(it, end, ec, t);
        if(ec == grammar::error::end)
        {
            // end of fields
            ec = {};
            it0 = it0 + (it - it0);
            break;
        }
        if(ec.failed())
            return it0;
        // remove obs-fold if needed
        if(t.v.has_obs_fold)
            replace_obs_fold(it0, it);
        it0 = it0 + (it - it0);
        auto const id =
            string_to_field(t.v.name);
        switch(id)
        {
        case field::connection:
        case field::proxy_connection:
            do_connection(
                t.v.value, ec);
            if(ec.failed())
                return it0;
            break;
        case field::content_length:
            do_content_length(
                t.v.value, ec);
            if(ec.failed())
                return it0;
            break;
        case field::transfer_encoding:
            do_transfer_encoding(
                t.v.value, ec);
            if(ec.failed())
                return it0;
            break;
        case field::upgrade:
            do_upgrade(t.v.value, ec);
            if(ec.failed())
                return it0;
            break;
        default:
            break;
        }
        auto& e =
            detail::fields_table(
            h_.buf + h_.cap)[h_.count];
        auto const base =
            h_.buf + h_.prefix;
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
        ++h_.count;
    }
    return it0;
}

void
parser::
parse_body(
    error_code& ec)
{
    ec = {};
    switch(state_)
    {
    case state::start_line:
    case state::header_fields:
        parse(ec);
        if(ec.failed())
            return;
        BOOST_ASSERT(state_ >
            state::header_fields);
        break;
    case state::body:
        break;
    case state::complete:
        ec = error::end_of_message;
        if(! got_eof_)
            return;
        state_ = state::end_of_stream;
        return;
    case state::end_of_stream:
        ec = error::end_of_stream;
        return;
    }

    auto avail = size_ - used_;

    if(m_.content_len.has_value())
    {
        // known payload length
        BOOST_ASSERT(! m_.got_chunked);
        BOOST_ASSERT(m_.n_remain > 0);
        BOOST_ASSERT(m_.content_len <
            cfg_.body_limit);
        if(avail == 0)
        {
            if(! got_eof_)
            {
                ec = grammar::error::incomplete;
                return;
            }
            ec = error::incomplete;
            return;
        }
        if( avail > m_.n_remain)
            avail = static_cast<
                std::size_t>(m_.n_remain);
        used_ += avail;
        m_.payload_seen += avail;
        m_.n_payload += avail;
        m_.n_remain -= avail;
        if(m_.n_remain > 0)
        {
            ec = {};
            return;
        }
        state_ = state::complete;
        ec = error::end_of_message;
        return;
    }

    if(! m_.got_chunked)
    {
        // end of body indicated by EOF
        if(avail > 0)
        {
            if(avail > (std::size_t(
                -1) - m_.n_payload))
            {
                // overflow size_t
                // VFALCO revisit this
                ec = error::numeric_overflow;
                return;
            }
            used_ += avail;
            m_.n_payload += avail;
            ec = {};
            return;
        }
        if(! got_eof_)
        {
            ec = grammar::error::incomplete;
            return;
        }
        state_ = state::complete;
        ec = error::end_of_message;
        return;
    }
#if 0
    if(m_.payload_left == 0)
    {
        // start of chunk
        bnf::chunk_part p;
        auto it = p.parse(
            h_.buf + used_,
            h_.buf + (
                size_ - used_),
            ec);
        if(ec)
            return;
        auto const v =
            p.value();
        m_.chunk.size = v.size;
        m_.chunk.ext = v.ext;
        m_.chunk.trailer = v.trailer;
        m_.chunk.fresh = true;
        m_.payload_left =
            v.size - v.data.size();
    }
    else
    {
        // continuation of chunk

    }
#endif
}

void
parser::
parse_chunk(
    error_code& ec)
{
    (void)ec;
#if 0
    switch(state_)
    {
    case state::start_line:
    case state::header_fields:
        parse_header(ec);
        if(ec.failed())
            return;
        BOOST_ASSERT(state_ >
            state::header_fields);
        break;
    case state::body:
        if(! m_.got_chunked)
            return parse_body(ec);
        break;
    case state::complete:
        ec = error::end_of_message;
        if(! got_eof_)
            return;
        state_ = state::end_of_stream;
        return;
    case state::end_of_stream:
        ec = error::end_of_stream;
        return;
    }

    auto const avail = size_ - used_;
    auto const start = h_.buf + used_;
    if(m_.payload_left == 0)
    {
        // start of chunk
        // VFALCO What about chunk_part_next?
        BOOST_ASSERT(
            used_ == m_.header_size);
        bnf::chunk_part p;
        auto it = p.parse(start,
            h_.buf + (
                size_ - used_), ec);
        BOOST_ASSERT(it == start);
        if(ec)
            return;
        auto const v = p.value();
        m_.chunk.size = v.size;
        m_.chunk.ext = v.ext;
        m_.chunk.fresh = true;
        if(v.size > 0)
        {
            // chunk
            m_.chunk.trailer = {};
            m_.payload_left =
                v.size - v.data.size();
            used_ += it - start; // excludes CRLF
            return;
        }
        // last-chunk
        BOOST_ASSERT(
            v.data.empty());
        m_.chunk.trailer =
            v.trailer;
        m_.body = {};
        used_ += it - start; // excludes CRLF
        state_ = state::complete;
    }
    else
    {
        // continuation of chunk

    }
#endif
}

//------------------------------------------------
//
// Special Members
//
//------------------------------------------------

void
parser::
discard_header() noexcept
{
    auto const n = h_.size;
    if(n == 0)
        return;
    if(state_ < state::body)
        return;
    size_ -= n;
    used_ -= n;
    std::memmove(
        h_.buf,
        h_.buf + n,
        size_);
    h_.count = 0;
    h_.prefix = 0;
    h_.size = 0;
    // VFALCO NOTE The field
    // table is also discarded...
}

// discard all of the body,
// but don't discard chunk-ext
void
parser::
discard_body() noexcept
{
    if(state_ != state::body)
        return;
    if(m_.n_payload == 0)
        return;
    size_ -= m_.n_payload;
    used_ -= m_.n_payload;
    if(! m_.got_chunked)
    {
        auto const n = h_.size;
        auto const pos = h_.buf + n;
        std::memmove(pos, pos + n,
            size_ - n - m_.n_payload);
        m_.n_payload = 0;
        return;
    }
    auto const n =
        h_.size +
        m_.n_chunk;
    auto const pos = h_.buf + n;
    std::memmove(pos, pos + n,
        size_ - n - m_.n_payload);
    m_.n_payload = 0;
}

// discard all of the payload
// including any chunk-ext
void
parser::
discard_chunk() noexcept
{
}

//------------------------------------------------

char*
parser::
parse_start_line(
    char* const start,
    char const* const end,
    error_code& ec) noexcept
{
    if(h_.kind == detail::kind::request)
    {
        request_line_rule t;
        char const* it = start;
        if(! grammar::parse(
            it, end, ec, t))
            return start;

        h_.version = t.v;
        h_.req.method = t.m;
        h_.req.method_len = static_cast<
            off_t>(t.ms.size());
        h_.req.target_len = static_cast<
            off_t>(t.t.size());
        return start + (it - start);
    }
    else
    {
        BOOST_ASSERT(h_.kind ==
            detail::kind::response);

        status_line_rule t;
        char const* it = start;
        if(! grammar::parse(
            it, end, ec, t))
            return start;

        h_.version = t.v;
        h_.res.status_int = t.status_int;
        return start + (it - start);
    }
}

void
parser::
finish_header(
    error_code& ec)
{
    if(h_.kind == detail::kind::request)
    {
        // https://tools.ietf.org/html/rfc7230#section-3.3
        if(m_.skip_body)
        {
            ec = error::end_of_message;
            state_ = state::end_of_message;
            return;
        }
        if(m_.content_len.has_value())
        {
            if( *m_.content_len > cfg_.body_limit)
            {
                ec = error::body_limit;
                return;
            }
            if(*m_.content_len > 0)
            {
                state_ = state::body;
                return;
            }
            ec = error::end_of_message;
            state_ = state::end_of_message;
            return;
        }
        else if(m_.got_chunked)
        {
            state_ = state::body;
            return;
        }
        ec = error::end_of_message;
        state_ = state::end_of_message;
    }
    else
    {
        BOOST_ASSERT(h_.kind ==
            detail::kind::response);

        // https://tools.ietf.org/html/rfc7230#section-3.3
        if((h_.res.status_int /  100 == 1) || // 1xx e.g. Continue
            h_.res.status_int == 204 ||       // No Content
            h_.res.status_int == 304)         // Not Modified
        {
            // Content-Length may be present, but we
            // treat the message as not having a body.
            m_.skip_body = true;
            return;
        }
        else if(m_.content_len.has_value())
        {
            if(*m_.content_len > 0)
            {
                //has_body_ = true;
                state_ = state::body;

                if( *m_.content_len > cfg_.body_limit)
                {
                    ec = error::body_limit;
                    return;
                }
            }
        }
    }
}

//------------------------------------------------

// https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
void
parser::
do_connection(
    string_view s, error_code& ec)
{
    (void)ec;

    for(auto v : bnf::range<bnf::connection>(s))
    {
        if(bnf::iequals(v, "close"))
        {
            m_.got_close = true;
        }
        else if(bnf::iequals(v, "keep-alive"))
        {
            m_.got_keep_alive = true;
        }
        else if(bnf::iequals(v, "upgrade"))
        {
            m_.got_upgrade = true;
        }
    }
}

void
parser::
do_content_length(
    string_view s,
    error_code& ec)
{
    // https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.2
    // Content-Length can be a comma separated
    // list, with all the same values, as well
    // as having multiple Content-Length fields
    // with the same number. We handle these here.
    auto list = bnf::range<
        bnf::list_of_one_or_more<
            bnf::dec_number>>(s);
    auto it = list.begin(ec);
    if(ec.failed())
    {
        ec = error::bad_content_length;
        return;
    }
    BOOST_ASSERT(it != list.end());
    auto v = *it;
    if(! m_.content_len.has_value())
    {
        if(v > cfg_.body_limit)
        {
            ec = error::body_limit;
            return;
        }
        m_.content_len = v;
    }
    else if(*m_.content_len != v)
    {
        // differing values
        ec = error::bad_content_length;
        return;
    }
    for(;;)
    {
        it.increment(ec);
        if(ec.failed())
        {
            ec = error::bad_content_length;
            return;
        }
        if(it == list.end())
            break;
        v = *it;
        if(m_.content_len != v)
        {
            // differing lengths
            ec = error::bad_content_length;
            return;
        }
    }
    if(! m_.skip_body)
        m_.n_remain = *m_.content_len;
}

void
parser::
do_transfer_encoding(
    string_view s, error_code& ec)
{
    using namespace detail::string_literals;

    bnf::range<bnf::transfer_encoding> te(s);
    auto const end = te.end();
    auto it = te.begin(ec);
    if(ec.failed())
    {
        // expected at least one encoding
        ec = error::bad_transfer_encoding;
        return;
    }
    BOOST_ASSERT(it != end);
    // handle multiple
    // Transfer-Encoding header lines
    m_.got_chunked = false;
    // get last encoding
    for(;;)
    {
        auto prev = it++;
        if(it == end)
        {
            if(bnf::iequals(
                prev->name, "chunked"))
                m_.got_chunked = true;
            break;
        }
    }
}

void
parser::
do_upgrade(
    string_view s, error_code& ec)
{
    (void)s;
    (void)ec;
}

} // http_proto
} // boost

#endif
