//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_PARSER_IPP

#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/detail/sv.hpp>
#include <boost/http_proto/bnf/chunk_part.hpp>
#include <boost/http_proto/bnf/connection.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>
#include <boost/http_proto/bnf/list.hpp>
#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/transfer_encoding.hpp>
#include <boost/assert.hpp>
#include <boost/none.hpp>
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------

parser::
config::
config() noexcept
    : header_limit(8192)
    , body_limit(4*1024*1024)
{
}

//------------------------------------------------

parser::
parser(
    context& ctx) noexcept
    : ctx_(ctx)
    , buffer_(nullptr)
    , cap_(0)
    , size_(0)
    , used_(0)
    , state_(state::start_line)
    , got_eof_(false)
    , m_{}
{
}

parser::
~parser()
{
    if(buffer_)
        delete[] buffer_;
}

chunk_info
parser::
chunk() const noexcept
{
    return m_.chunk;
}

string_view
parser::
payload() const noexcept
{
    // VFALCO How about some
    // asserts or exceptions?
    if(! m_.got_chunked)
        return string_view(
            buffer_ +
                m_.n_header,
            m_.n_payload);
    return string_view(
        buffer_ +
            m_.n_header +
            m_.n_chunk,
        m_.n_payload);
}

//------------------------------------------------

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
        BOOST_ASSERT(state_ ==
            state::end_of_message);
        if( size_ > used_ &&
            used_ > 0)
        {
            // move unused octets to front
            std::memcpy(
                buffer_,
                buffer_ + used_,
                size_ - used_);
            size_ -= used_;
            used_ = 0;
        }
    }

    m_ = message();
    state_ = state::start_line;
}

std::pair<void*, std::size_t>
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
    if(! buffer_)
    {
        // VFALCO This should be
        // configurable or something
        buffer_ = new char[4096];
        cap_ = 4096;
    }
    else if(cap_ <= size_)
    {
        // VFALCO Put a configurable
        // growth policy here.
        std::size_t amount = 4096;
        auto buffer = new char[
            cap_ + amount];
        std::memcpy(buffer,
            buffer_, used_);
        delete[] buffer_;
        buffer_ = buffer;
        cap_ =
            cap_ + amount;
    }
    return {
        buffer_ + size_,
        cap_ - size_ };
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
    BOOST_ASSERT(n <= cap_ - size_);
    size_ += n;
}

void
parser::
commit_eof()
{
    got_eof_ = true;
}

void
parser::
discard_header() noexcept
{
    if(m_.n_header == 0)
        return;
    if(state_ < state::payload)
        return;
    size_ -= m_.n_header;
    used_ -= m_.n_header;
    std::memmove(
        buffer_,
        buffer_ + m_.n_header,
        size_);
    m_.n_header = 0;
}

// discard all of the payload,
// but don't discard chunk-ext
void
parser::
discard_payload() noexcept
{
    if(state_ != state::payload)
        return;
    if(m_.n_payload == 0)
        return;
    size_ -= m_.n_payload;
    used_ -= m_.n_payload;
    if(! m_.got_chunked)
    {
        auto const n = m_.n_header;
        auto const pos = buffer_ + n;
        std::memmove(pos, pos + n,
            size_ - n - m_.n_payload);
        m_.n_payload = 0;
        return;
    }
    auto const n =
        m_.n_header + m_.n_chunk;
    auto const pos = buffer_ + n;
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

void
parser::
parse_header(
    error_code& ec)
{
    switch(state_)
    {
    case state::start_line:
    {
        BOOST_ASSERT(used_ == 0);
        BOOST_ASSERT(
            m_.n_header == 0);
        std::size_t n;
        auto const limit =
            size_ > cfg_.header_limit;
        if(! limit)
            n = size_;
        else
            n = cfg_.header_limit;
        ec = {};
        auto it = parse_start_line(
            buffer_, buffer_ + n, ec);
        used_ += it - buffer_;
        if(ec == error::need_more)
        {
            if(! limit)
                return;
            ec = error::header_limit;
            return;
        }
        if(ec.failed())
            return;
        m_.n_header += it - buffer_;
        state_ = state::header_fields;
        BOOST_FALLTHROUGH;
    }
    case state::header_fields:
    {
        std::size_t n;
        auto const limit =
            size_ > cfg_.header_limit;
        if(! limit)
            n = size_;
        else
            n = cfg_.header_limit;
        BOOST_ASSERT(
            n >= m_.n_header);
        ec = {};
        auto start =
            buffer_ + m_.n_header;
        auto it = parse_fields(
            start, buffer_ + n, ec);
        used_ += it - start;
        if(ec == error::need_more)
        {
            if(! limit)
                return;
            ec = error::header_limit;
            return;
        }
        if(ec.failed())
            return;
        m_.n_header += it - start;
        break;
    }
    case state::payload:
    {
        ec = {};
        return;
    }
    case state::end_of_message:
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
        state_ = state::payload;
    }
    else
    {
        state_ = state::end_of_message;
    }
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
        parse_header(ec);
        if(ec.failed())
            return;
        BOOST_ASSERT(state_ >
            state::header_fields);
        break;
    case state::payload:
        break;
    case state::end_of_message:
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

    if(m_.got_content_length)
    {
        // known payload length
        BOOST_ASSERT(! m_.got_chunked);
        BOOST_ASSERT(m_.n_remain > 0);
        BOOST_ASSERT(m_.content_length <
            cfg_.body_limit);
        if(avail == 0)
        {
            if(! got_eof_)
            {
                ec = error::need_more;
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
        state_ = state::end_of_message;
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
            ec = error::need_more;
            return;
        }
        state_ = state::end_of_message;
        ec = error::end_of_message;
        return;
    }
#if 0
    if(m_.payload_left == 0)
    {
        // start of chunk
        bnf::chunk_part p;
        auto it = p.parse(
            buffer_ + used_,
            buffer_ + (
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
    case state::payload:
        if(! m_.got_chunked)
            return parse_body(ec);
        break;
    case state::end_of_message:
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
    auto const start = buffer_ + used_;
    if(m_.payload_left == 0)
    {
        // start of chunk
        // VFALCO What about chunk_part_next?
        BOOST_ASSERT(
            used_ == m_.header_size);
        bnf::chunk_part p;
        auto it = p.parse(start,
            buffer_ + (
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
        state_ = state::end_of_message;
    }
    else
    {
        // continuation of chunk

    }
#endif
}

//------------------------------------------------

char*
parser::
parse_fields(
    char* const start,
    char const* const end,
    error_code& ec)
{
    bnf::header_fields p;
    auto cit = p.begin(
        start, end, ec);
    auto it = start;
    for(;;)
    {
        if(ec == error::end)
        {
            ec = {};
            break;
        }
        if(ec.failed())
            return start + (
                cit - start);
        if(p.value().has_obs_fold)
            bnf::replace_obs_fold(it, cit);
        it = start + (cit - start);
        auto const f =
            string_to_field(
                p.value().name);
        switch(f)
        {
        case field::connection:
        case field::proxy_connection:
            do_connection(
                p.value().value, ec);
            if(ec.failed())
                return it;
            break;
        case field::content_length:
            do_content_length(
                p.value().value, ec);
            if(ec.failed())
                return it;
            break;
        case field::transfer_encoding:
            do_transfer_encoding(
                p.value().value, ec);
            if(ec.failed())
                return it;
            break;
        case field::upgrade:
            do_upgrade(
                p.value().value, ec);
            if(ec.failed())
                return it;
            break;
        default:
            break;
        }
        cit = p.increment(cit, end, ec);
    }
    return it;
}

//------------------------------------------------

// https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
void
parser::
do_connection(
    string_view s, error_code& ec)
{
    (void)ec;

    using namespace detail::string_literals;
    for(auto v : bnf::range<bnf::connection>(s))
    {
        if(bnf::iequals(v, "close"_sv))
        {
            m_.got_close = true;
        }
        else if(bnf::iequals(v, "keep-alive"_sv))
        {
            m_.got_keep_alive = true;
        }
        else if(bnf::iequals(v, "upgrade"_sv))
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
    if(! m_.got_content_length)
    {
        if(v > cfg_.body_limit)
        {
            ec = error::body_limit;
            return;
        }
        m_.content_length = v;
        m_.got_content_length = true;
    }
    else if(m_.content_length != v)
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
        if(m_.content_length != v)
        {
            // differing lengths
            ec = error::bad_content_length;
            return;
        }
    }
    if(! m_.skip_body)
        m_.n_remain = m_.content_length;
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
                it->name, "chunked"_sv))
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
