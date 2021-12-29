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
#include <boost/http_proto/detail/ftab.hpp>
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
    , buf_(nullptr)
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
    if(buf_)
        delete[] buf_;
}

chunk_info
parser::
chunk() const noexcept
{
    return m_.chunk;
}

string_view
parser::
body() const noexcept
{
    // VFALCO How about some
    // asserts or exceptions?
    if(! m_.got_chunked)
        return string_view(
            buf_ +
                m_.start_len +
                m_.fields_len,
            m_.n_payload);
    return string_view(
        buf_ +
            m_.start_len +
            m_.fields_len +
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
                buf_,
                buf_ + used_,
                size_ - used_);
            size_ -= used_;
            used_ = 0;
        }
    }

    m_ = message{};
    state_ = state::start_line;
}

mutable_buffer
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
    if(! buf_)
    {
        // VFALCO This should be
        // configurable or something
        buf_ = new char[4096];
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
            buf_, used_);
        delete[] buf_;
        buf_ = buffer;
        cap_ =
            cap_ + amount;
    }
    return {
        buf_ + size_,
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
    auto const n =
        m_.start_len +
        m_.fields_len;
    if(n == 0)
        return;
    if(state_ < state::body)
        return;
    size_ -= n;
    used_ -= n;
    std::memmove(
        buf_,
        buf_ + n,
        size_);
    m_.count = 0;
    m_.start_len = 0;
    m_.fields_len = 0;
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
        auto const n =
            m_.start_len +
            m_.fields_len;
        auto const pos = buf_ + n;
        std::memmove(pos, pos + n,
            size_ - n - m_.n_payload);
        m_.n_payload = 0;
        return;
    }
    auto const n =
        m_.start_len +
        m_.fields_len +
        m_.n_chunk;
    auto const pos = buf_ + n;
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
        BOOST_ASSERT(m_.start_len == 0);
        BOOST_ASSERT(m_.fields_len == 0);
        if(! buf_)
        {
            ec = grammar::error::incomplete;
            return;
        }
        std::size_t n;
        bool const limit = size_ >
            cfg_.header_limit;
        if(! limit)
            n = size_;
        else
            n = cfg_.header_limit;
        ec = {};
        auto it = parse_start_line(
            buf_, buf_ + n, ec);
        used_ += it - buf_;
        if(ec == grammar::error::incomplete)
        {
            if(! limit)
                return;
            ec = error::header_limit;
            return;
        }
        if(ec.failed())
            return;
        m_.start_len = it - buf_;
        state_ = state::header_fields;
        BOOST_FALLTHROUGH;
    }
    case state::header_fields:
    {
        std::size_t n;
        bool const limit = size_ >
            cfg_.header_limit;
        if(! limit)
            n = size_;
        else
            n = cfg_.header_limit;
        BOOST_ASSERT(
            m_.start_len +
            m_.fields_len <
                cfg_.header_limit);
        ec = {};
        auto start = buf_ +
            m_.start_len +
            m_.fields_len;
        auto it = parse_fields(
            start, buf_ + n, ec);
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
        m_.fields_len = (it - buf_) -
            m_.start_len;
        break;
    }
    case state::body:
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
        state_ = state::body;
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
    case state::body:
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
            ec = grammar::error::incomplete;
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
            buf_ + used_,
            buf_ + (
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
    auto const start = buf_ + used_;
    if(m_.payload_left == 0)
    {
        // start of chunk
        // VFALCO What about chunk_part_next?
        BOOST_ASSERT(
            used_ == m_.header_size);
        bnf::chunk_part p;
        auto it = p.parse(start,
            buf_ + (
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
    field_rule t;
    char* it0 = start;
    char const* it = start;
    for(;;)
    {
        parse(it, end, ec, t);
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
        auto ft = detail::get_ftab(
            buf_ + cap_);
        auto& fi = ft[m_.count];
        fi.id = id;
        fi.pos = static_cast<
            off_t>(start - buf_);
        fi.name_pos = static_cast<
            off_t>(t.v.name.data() - buf_);
        fi.name_len = static_cast<
            off_t>(t.v.name.size());
        fi.value_len = static_cast<
            off_t>(t.v.value.size());
        if(fi.value_len > 0)
            fi.value_pos = static_cast<
                off_t>(t.v.value.data() - buf_);
        else
            fi.value_pos = 0; // empty string
        ++m_.count;
    }
    return it0;
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
