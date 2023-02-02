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
#include <boost/http_proto/detail/codec.hpp>
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
    // headers_limit too large
    if( cfg_.headers_limit >
        BOOST_HTTP_PROTO_MAX_HEADER)
        detail::throw_invalid_argument();

    // start_line_limit too large
    if( cfg_.start_line_limit >=
        cfg_.headers_limit)
        detail::throw_invalid_argument();

    // field_size_limit too large
    if( cfg_.field_size_limit >=
        cfg_.headers_limit)
        detail::throw_invalid_argument();

    // fields_limit too large
    if( cfg_.fields_limit >
        cfg_.headers_limit / 4)
        detail::throw_invalid_argument();

    // largest space needed
    auto const bytes_needed =
        detail::header::bytes_needed(
            cfg_.headers_limit,
            cfg_.fields_limit);

    // prevent overflow
    if(extra_buffer_size >
            std::size_t(-1) - bytes_needed)
        detail::throw_invalid_argument();

    // allocate max headers plus extra
    ws_ = detail::workspace(
        bytes_needed +
        extra_buffer_size);

    h_.cap = bytes_needed;

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
// Modifiers
//
//------------------------------------------------

// prepare for a new stream
void
parser::
reset() noexcept
{
    st_ = state::need_start;
    got_eof_ = false;
}

void
parser::
start_impl(
    bool head_response)
{
    std::size_t initial_size = 0;
    switch(st_)
    {
    default:
    case state::need_start:
        BOOST_ASSERT(h_.size == 0);
        BOOST_ASSERT(h_buf_.size() == 0);
        BOOST_ASSERT(! got_eof_);
        break;

    case state::headers:
        // Can't call start twice.
        detail::throw_logic_error();

    case state::headers_done:
    case state::body:
        // Can't call start with
        // an incomplete message.
        detail::throw_logic_error();

    case state::complete:
        if(h_buf_.size() > 0)
        {
            // headers with no body
            BOOST_ASSERT(h_.size > 0);
            h_buf_.consume(h_.size);
            initial_size = h_buf_.size();
            // move unused octets to front
            buffer_copy(
                mutable_buffer(
                    ws_.data(),
                    initial_size),
                h_buf_.data());
        }
        else
        {
            // leftover data after body
        }
        break;
    }

    ws_.clear();

    // set up header read buffer
    h_buf_ = {
        ws_.data(),
        cfg_.headers_limit,
        initial_size };

    // reset the header but
    // preserve the capacity
    auto const cap = h_.cap;
    h_ = detail::header(
        detail::empty{h_.kind});
    h_.buf = reinterpret_cast<
        char*>(ws_.data());
    h_.cbuf = h_.buf;
    h_.cap = cap;

    cfg_impl_ = {};
    cfg_impl_.headers_limit = cfg_.headers_limit;
    cfg_impl_.start_line_limit = cfg_.start_line_limit;
    cfg_impl_.field_size_limit = cfg_.field_size_limit;
    cfg_impl_.fields_limit = cfg_.fields_limit;

    st_ = state::headers;

    BOOST_ASSERT(! head_response ||
        h_.kind == detail::kind::response);
    head_response_ = head_response;
}

auto
parser::
prepare() ->
    mutable_buffers_type
{
    switch(st_)
    {
    default:
    case state::need_start:
        // start must be called once
        // before calling prepare.
        detail::throw_logic_error();

    case state::headers:
        // fill up to headers_limit
        return {
            h_buf_.prepare(
                cfg_.headers_limit -
                h_buf_.size()),
            mutable_buffer{} };

    case state::headers_done:
    {
        // discard headers and move
        // any leftover stream data.
        std::memmove(
            ws_.data(),
            h_.cbuf + h_.size,
            h_buf_.size() - h_.size);
        st_ = state::body;
        // VFALCO set up body buffer
        BOOST_FALLTHROUGH;
    }

    case state::body:
    {
        return {};
    }

    case state::complete:
        // Can't call `prepare` again after
        // a complete message is parsed,
        // call `start` first.
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

    switch(st_)
    {
    default:
    case state::need_start:
    case state::headers:
        h_buf_.commit(n);
        break;

    case state::headers_done:
    case state::body:
    case state::complete:
        break;
    }
}

void
parser::
commit_eof()
{
    switch(st_)
    {
    default:
    case state::need_start:
        // Can't commit eof
        // before calling start.
        detail::throw_logic_error();

    case state::headers:
    case state::headers_done:
    case state::body:
        got_eof_ = true;
        break;

    case state::complete:
        // Can't commit eof when
        // message is complete.
        detail::throw_logic_error();
    }
}

// process input data then
// eof if input data runs out.
void
parser::
parse(
    error_code& ec)
{
    switch(st_)
    {
    default:
    case state::need_start:
        // You must call start before
        // calling parse on a new message.
        detail::throw_logic_error();

    case state::headers:
    {
        BOOST_ASSERT(h_.buf == ws_.data());
        BOOST_ASSERT(h_.cbuf == ws_.data());
        auto const new_size = h_buf_.size();
        h_.parse(cfg_impl_, new_size, ec);
        if(! ec.failed())
        {
            if( h_.md.payload != payload::none &&
                ! head_response_)
            {
                // Deliver headers to caller
                st_ = state::headers_done;
                break;
            }
            // no payload
            st_ = state::complete;
            break;
        }
        if( ec == grammar::error::need_more &&
            got_eof_)
        {
            if(h_.size > 0)
            {
                // Connection closed before
                // message is complete.
                ec = BOOST_HTTP_PROTO_ERR(
                    error::incomplete);

                return;
            }

            // Connection closed
            // cleanly.
            ec = BOOST_HTTP_PROTO_ERR(
                error::end_of_stream);

            return;
        }
        return;
    }

    case state::headers_done:
    {
        // This is a no-op
        ec = {};
        break;
    }

    case state::body:
    {
        parse_body(ec);
        if(ec.failed())
            return;
        st_ = state::complete;
        break;
    }
    }
}

//------------------------------------------------

string_view
parser::
release_buffered_data() noexcept
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

auto
parser::
safe_get_header() const ->
    detail::header const*
{
    switch(st_)
    {
    default:
    case state::need_start:
    case state::headers:
        // Headers not received yet
        detail::throw_logic_error();

    case state::headers_done:
        break;

    case state::body:
        // Headers received and discarded
        detail::throw_logic_error();

    case state::complete:
        // VFALCO Could be OK
        break;
    }
    return &h_;
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
