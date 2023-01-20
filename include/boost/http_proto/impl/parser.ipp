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
#include <boost/http_proto/detail/except.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/assert.hpp>
#include <boost/none.hpp>
#include <memory>

namespace boost {
namespace http_proto {

/*
    Parser design:

    The usage of the parser is thus:

    pr.reset(); // prepare for a new stream

    pr.start(); // prepare for a new message
    pr.start_head_response(); // new message with no payload

    read_header( ..., pr );
        do
        {
            read_some(..., pr );
        }
        while(! got_header());

    pr.set_body( ... );
        // invalidates the headers? yes.

    read_body( ..., pr );
        while(! (pr.flags() &
            parser::is_done_bit ) );
        {
            read_some(..., pr );
        }

    If these are called out of order, an
    exception is thrown.

    Every call to `prepare` must be
    followed by a call to commit, reset,
    or the destructor.
*/
//------------------------------------------------

parser::
parser(
    detail::kind k,
    config_base const& cfg)
    : cfg_(cfg)
    , h_(detail::empty{k})
{
}

void
parser::
construct(
    std::size_t extra_buffer_size)
{
    // max_headers_size too large
    if( cfg_.max_headers_size >
        BOOST_HTTP_PROTO_MAX_HEADER)
        detail::throw_invalid_argument();

    // max_field_size too large
    if( cfg_.max_field_size >=
        cfg_.max_headers_size)
        detail::throw_invalid_argument();

    // max_field_count too large
    if( cfg_.max_field_count >
        cfg_.max_headers_size / 4)
        detail::throw_invalid_argument();

    // largest space needed
    auto const bytes_needed =
        detail::header::bytes_needed(
            cfg_.max_headers_size,
            cfg_.max_field_count);

    // extra_buffer_size is too large
    if(extra_buffer_size >
            std::size_t(-1) - bytes_needed)
        detail::throw_invalid_argument();

/*  Layout:

    |<- bytes_needed ->|<- extra_buffer_size ->|
*/
    ws_ = detail::workspace(
        bytes_needed +
        extra_buffer_size);

    reset();
}

//------------------------------------------------
//
// Special Members
//
//------------------------------------------------

parser::
~parser()
{
}

parser::
parser(
    parser&&) noexcept = default;

//------------------------------------------------
//
// Observers
//
//------------------------------------------------

string_view
parser::
body() const noexcept
{
#if 0
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
#else
    return {};
#endif
}

//------------------------------------------------
//
// Input
//
//------------------------------------------------

void
parser::
reset() noexcept
{
    // largest space needed
    auto const bytes_needed =
        detail::header::bytes_needed(
            cfg_.max_headers_size,
            cfg_.max_field_count);
    h_ = detail::header(
        detail::empty{h_.kind});
    h_.buf = reinterpret_cast<
        char*>(ws_.data());
    h_.cbuf = h_.buf;
    h_.cap = bytes_needed;

    st_ = state::need_start;

    m_ = {};

    got_eof_ = false;
}

auto
parser::
prepare() ->
    buffers
{
    // Forgot to call start
    if(st_ == state::need_start)
        detail::throw_logic_error();

    // Can't prepare after eof
    if(got_eof_)
        detail::throw_logic_error();

    switch(st_)
    {
    case state::need_start:
    case state::start_line:
    case state::fields:
    {
        // read up to max_headers_size
        auto const n =
            cfg_.max_headers_size -
            rd_buf_.size();
        return {
            rd_buf_.prepare(n),
            mutable_buffer{} };
    }

    case state::headers:
    {
        // discard headers and move
        // any leftover stream data.
        BOOST_FALLTHROUGH;
    }

    case state::body:
    {
        return {};
    }

    default:
    case state::complete:
        // Can't call prepare again after
        // a complete message is parsed.
        detail::throw_logic_error();
    }
}

void
parser::
commit(
    std::size_t n)
{
    // Can't commit after eof
    if(got_eof_)
        detail::throw_logic_error();

    rd_buf_.commit(n);
}

void
parser::
commit_eof()
{
    // Can't commit eof twice
    if(got_eof_)
        detail::throw_logic_error();

    got_eof_ = true;

    switch(st_)
    {
    default:
    case state::need_start:
    case state::start_line:
    case state::fields:
    case state::headers:
    case state::body:
    case state::complete:
        break;
    }
}

void
parser::
parse(
    error_code& ec)
{
    ec.clear();
    while(st_ != state::complete)
    {
        switch(st_)
        {
        case state::need_start:
            if(got_eof_)
            {
                BOOST_ASSERT(h_.size == 0);
                ec = error::end_of_stream;
                return;
            }
            st_ = state::start_line;
            BOOST_FALLTHROUGH;

        case state::start_line:
            parse_start_line(ec);
            if(ec.failed())
                return;
            st_ = state::fields;
            BOOST_FALLTHROUGH;

        case state::fields:
            parse_fields(ec);
            if(ec.failed())
                return;
            st_ = state::body;
            BOOST_FALLTHROUGH;

        case state::body:
            parse_body(ec);
            if(ec.failed())
                return;
            st_ = state::complete;
            BOOST_FALLTHROUGH;

        default:
            break;
        }
    }
}

//------------------------------------------------
//
//
//
//------------------------------------------------

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

string_view
parser::
buffered_data() noexcept
{
    return {};
}

//------------------------------------------------
//
// Implementation
//
//------------------------------------------------

void
parser::
apply_param(
    config_base const& cfg) noexcept
{
    cfg_ = cfg;
}

//------------------------------------------------

void
parser::
start_impl(bool head_response)
{
    // Can't call start after
    // EOF, call reset instead.
    if(got_eof_)
        detail::throw_logic_error();

    // Can't call start with an
    // incomplete current message.
    if( st_ != state::complete &&
        st_ != state::need_start)
        detail::throw_logic_error();

/*
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
    else
    {
        committed_ = 0;
    }
*/

    // reset the header but
    // preserve the capacity
    detail::header h(
        detail::empty{h_.kind});
    h.assign_to(h_);

    m_ = message{};
    st_ = state::start_line;

    BOOST_ASSERT(! head_response ||
        h_.kind == detail::kind::response);
    head_response_ = head_response;



    // put leftovers at the beginning
    // of the buffer

    ws_.clear();

    BOOST_ASSERT(ws_.size() >=
        cfg_.max_headers_size);
    rd_buf_ = {
        ws_.data(),
        cfg_.max_headers_size };
}

//------------------------------------------------

void
parser::
parse_start_line(
    error_code& ec)
{
    BOOST_ASSERT(h_.buf == ws_.data());
    BOOST_ASSERT(h_.cbuf == ws_.data());
    auto const new_size = rd_buf_.size();
    h_.parse_start_line(new_size, ec);
    if(! ec.failed())
        return;
    if(ec == grammar::error::need_more)
    {
        if( new_size <
            cfg_.max_headers_size)
            return;
        ec = BOOST_HTTP_PROTO_ERR(
            error::header_too_large);
        return;
    }
    return;
}

void
parser::
parse_fields(
    error_code& ec)
{
    for(;;)
    {
        auto const size0 = h_.size;
        auto got_one = h_.parse_field(
            rd_buf_.size(), ec);
        if(ec == grammar::error::need_more)
        {
            if(rd_buf_.size() >=
                    cfg_.max_headers_size)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::header_too_large);
                return;
            }
        }
        if(! got_one)
            return;
        if(h_.count > cfg_.max_field_count)
        {
            ec = BOOST_HTTP_PROTO_ERR(
                error::too_many_fields);
            return;
        }
        if(h_.size >
            cfg_.max_field_size + size0)
        {
            ec = BOOST_HTTP_PROTO_ERR(
                error::field_too_large);
            return;
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
    BOOST_ASSERT(st_ == state::body);

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
        st_ = state::complete;
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
        st_ = state::complete;
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
    switch(st_)
    {
    case state::start_line_line:
    case state::header_fields:
        parse_header(ec);
        if(ec.failed())
            return;
        BOOST_ASSERT(st_ >
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
        st_ = state::end_of_stream;
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
        st_ = state::complete;
    }
    else
    {
        // continuation of chunk

    }
#endif
}

} // http_proto
} // boost

#endif
