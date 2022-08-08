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
#include <boost/url/grammar/ci_string.hpp>
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
    , committed_(0)
    , state_(state::empty)
    , got_eof_(false)
    , m_{}
{
    h_.kind = k;
    // buffer must be large enough to
    // hold a complete header.
    if( buffer_bytes <
        cfg.max_header_size)
        buffer_bytes =
            cfg.max_header_size;

    h_.buf = new char[buffer_bytes];
    h_.cbuf = h_.buf;
    h_.cap = buffer_bytes;
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
    reset();
}

void
parser::
reset()
{
    if(got_eof_)
    {
        // new connection, throw
        // out all previous state.
        committed_ = 0;
        got_eof_ = false;
    }
    else
    {
        // Throwing out partial data
        // will desync the HTTP stream
        //BOOST_ASSERT(is_complete());
        if( committed_ > h_.size &&
            h_.size > 0)
        {
            // move unused octets to front
            std::memcpy(
                h_.buf,
                h_.buf + h_.size,
                committed_ - h_.size);
            committed_ -= h_.size;
            h_.size = 0;
        }
    }

    // reset the header but
    // preserve the capacity
    detail::header h;
    h.kind = h_.kind;
    h.assign_to(h_);

    m_ = message{};
    state_ = state::empty;
}

mutable_buffers
parser::
prepare()
{
    // Can't commit bytes after eof
    BOOST_ASSERT(! got_eof_);

    mb_ = {
        h_.buf + committed_,
        h_.cap - committed_ };
    return { &mb_, 1 };
}

void
parser::
commit(
    std::size_t n,
    error_code& ec)
{
    BOOST_ASSERT(! got_eof_);
    BOOST_ASSERT(
        n <= h_.cap - committed_);
    committed_ += n;
    parse(ec);
}

void
parser::
commit_eof(
    error_code &ec)
{
    got_eof_ = true;
    parse(ec);
}

//------------------------------------------------
//
//
//
//------------------------------------------------

void
parser::
discard_header() noexcept
{
}

// discard all of the body,
// but don't discard chunk-ext
void
parser::
discard_body() noexcept
{
}

// discard all of the payload
// including any chunk-ext
void
parser::
discard_chunk() noexcept
{
}

//------------------------------------------------
//
// Implementation
//
//------------------------------------------------

void
parser::
parse(
    error_code& ec)
{
    ec.clear();
    while(state_ != state::complete)
    {
        switch(state_)
        {
        case state::empty:
            if(got_eof_)
            {
                BOOST_ASSERT(h_.size == 0);
                ec = error::end_of_stream;
                return;
            }
            state_ = state::start_line;
            BOOST_FALLTHROUGH;

        case state::start_line:
            parse_start_line(ec);
            if(ec.failed())
                return;
            state_ = state::header_fields;
            BOOST_FALLTHROUGH;

        case state::header_fields:
            parse_fields(ec);
            if(ec.failed())
                return;
            state_ = state::body;
            BOOST_FALLTHROUGH;

        case state::body:
            parse_body(ec);
            if(ec.failed())
                return;
            state_ = state::complete;
            BOOST_FALLTHROUGH;

        default:
            break;
        }
    }
}

void
parser::
parse_start_line(
    error_code& ec)
{
    BOOST_ASSERT(h_.size == 0);
    std::size_t const new_size =
        committed_ <= cfg_.max_header_size
        ? committed_
        : cfg_.max_header_size;
    detail::parse_start_line(
        h_, new_size, ec);
    if(ec ==
        grammar::error::need_more)
    {
        if( new_size <
            cfg_.max_header_size)
            return;
        ec = error::header_too_large;
        return;
    }
    if(ec.failed())
        return;
}

void
parser::
parse_fields(
    error_code& ec)
{
    std::size_t const new_size =
        committed_ <= cfg_.max_header_size
        ? committed_
        : cfg_.max_header_size;
    field id;
    string_view v;
    for(;;)
    {
        auto const size0 = h_.size;
        auto got_one = detail::parse_field(
            h_, new_size, id, v, ec);
        if(ec == grammar::error::need_more)
        {
            if(new_size >=
                    cfg_.max_header_size)
                ec = error::header_too_large;
        }
        if(! got_one)
            return;
        if(h_.count > cfg_.max_field_count)
        {
            ec = error::too_many_fields;
            return;
        }
        if(h_.size >
            cfg_.max_field_size + size0)
        {
            ec = error::field_too_large;
            return;
        }
        switch(id)
        {
        case field::connection:
        case field::proxy_connection:
            do_connection(v, ec);
            if(ec.failed())
                return;
            break;
        case field::content_length:
            do_content_length(v, ec);
            if(ec.failed())
                return;
            break;
        case field::transfer_encoding:
            do_transfer_encoding(v, ec);
            if(ec.failed())
                return;
            break;
        case field::upgrade:
            do_upgrade(v, ec);
            if(ec.failed())
                return;
            break;
        default:
            break;
        }
    }
}

void
parser::
parse_body(
    error_code& ec)
{
    (void)ec;
return;
// VFALCO TODO
#if 0
    BOOST_ASSERT(state_ == state::body);

    if(h_.kind == detail::kind::request)
    {
        // https://tools.ietf.org/html/rfc7230#section-3.3
        if(m_.skip_body)
            return;
        if(m_.content_len.has_value())
        {
            if(*m_.content_len > cfg_.body_too_large)
            {
                ec = error::body_too_large;
                return;
            }
            if(*m_.content_len == 0)
                return;
        }
        else if(m_.got_chunked)
        {
            // VFALCO TODO
            return;
        }
        else
        {
            // Content-Length: 0
            return;
        }
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
        }
        else if(m_.content_len.has_value())
        {
            if(*m_.content_len > 0)
            {
                if(*m_.content_len > cfg_.body_too_large)
                {
                    ec = error::body_too_large;
                    return;
                }
            }
        }
        else
        {
            // No Content-Length
            return;
        }
    }

    auto avail = committed_ - size_;
    if(m_.content_len.has_value())
    {
        // known payload length
        BOOST_ASSERT(! m_.got_chunked);
        BOOST_ASSERT(m_.n_remain > 0);
        BOOST_ASSERT(m_.content_len <
            cfg_.body_too_large);
        if(avail == 0)
        {
            if(! got_eof_)
            {
                ec = grammar::error::need_more;
                return;
            }
            ec = error::need_more;
            return;
        }
        if( avail > m_.n_remain)
            avail = static_cast<
                std::size_t>(m_.n_remain);
        size_ += avail;
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
            size_ += avail;
            m_.n_payload += avail;
            ec = {};
            return;
        }
        if(! got_eof_)
        {
            ec = grammar::error::need_more;
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
            h_.buf + size_,
            h_.buf + (
                committed_ - size_),
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

    auto const avail = committed_ - size_;
    auto const start = h_.buf + size_;
    if(m_.payload_left == 0)
    {
        // start of chunk
        // VFALCO What about chunk_part_next?
        BOOST_ASSERT(
            size_ == m_.header_size);
        bnf::chunk_part p;
        auto it = p.parse(start,
            h_.buf + (
                committed_ - size_), ec);
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
            size_ += it - start; // excludes CRLF
            return;
        }
        // last-chunk
        BOOST_ASSERT(
            v.data.empty());
        m_.chunk.trailer =
            v.trailer;
        m_.body = {};
        size_ += it - start; // excludes CRLF
        state_ = state::complete;
    }
    else
    {
        // continuation of chunk

    }
#endif
}

//------------------------------------------------

// https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
void
parser::
do_connection(
    string_view s, error_code& ec)
{
    (void)ec;

#if 0
    for(auto v : bnf::range<bnf::connection>(s))
    {
        if(grammar::ci_is_equal(v, "close"))
        {
            m_.got_close = true;
        }
        else if(grammar::ci_is_equal(v, "keep-alive"))
        {
            m_.got_keep_alive = true;
        }
        else if(grammar::ci_is_equal(v, "upgrade"))
        {
            m_.got_upgrade = true;
        }
    }
#endif
}

void
parser::
do_content_length(
    string_view s,
    error_code& ec)
{
#if 0
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
        if(v > cfg_.max_body_size)
        {
            ec = error::body_too_large;
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
#endif
}

void
parser::
do_transfer_encoding(
    string_view s, error_code& ec)
{
#if 0
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
            if(grammar::ci_is_equal(
                prev->name, "chunked"))
                m_.got_chunked = true;
            break;
        }
    }
#endif
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
