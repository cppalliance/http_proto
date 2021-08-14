//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_BASIC_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_BASIC_PARSER_IPP

#include <boost/http_proto/basic_parser.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/detail/sv.hpp>
#include <boost/http_proto/bnf/chunk_part.hpp>
#include <boost/http_proto/bnf/connection.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>
#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/transfer_encoding.hpp>
#include <boost/assert.hpp>
#include <boost/none.hpp>
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------

constexpr
basic_parser::
config::
config() noexcept
    : header_limit(8192)
    , body_limit(4*1024*1024)
{
}

//------------------------------------------------

basic_parser::
basic_parser(
    context& ctx) noexcept
    : ctx_(ctx)
    , buffer_(nullptr)
    , cap_(0)
    , size_(0)
    , used_(0)
    , state_(state::nothing_yet)
    , got_eof_(false)
    , m_{}
{
}

basic_parser::
~basic_parser()
{
    if(buffer_)
        delete[] buffer_;
}

void
basic_parser::
reset()
{
    // move leftovers to front
    if( size_ > used_ &&
        used_ > 0)
    {
        std::memcpy(
            buffer_,
            buffer_ + used_,
            size_ - used_);
        size_ -= used_;
        used_ = 0;
    }

    m_ = message();
}

std::pair<void*, std::size_t>
basic_parser::
prepare()
{
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
basic_parser::
commit(
    std::size_t n)
{
    BOOST_ASSERT(n <=
        cap_ - size_);
    size_ += n;
}

void
basic_parser::
commit_eof()
{
}

void
basic_parser::
discard_header() noexcept
{
    if(m_.header_size == 0)
        return;
}

// discard all of the payload,
// but don't discard chunk-ext
void
basic_parser::
discard_body() noexcept
{

}

// discard all of the payload
// including any chunk-ext
void
basic_parser::
discard_chunk() noexcept
{
}

//------------------------------------------------

void
basic_parser::
parse_header(
    error_code& ec)
{
    switch(state_)
    {
    case state::body:
        ec = {};
        return;
    case state::end_of_message:
        ec = error::end_of_message;
        return;
    case state::header:
        break;
    }

    BOOST_ASSERT(used_ == 0);
    auto const start = buffer_;
    auto avail = size_;
    if( avail > cfg_.header_limit)
        avail = cfg_.header_limit;
    auto const end = buffer_ + avail;

    // start-line
    auto it = parse_start_line(
        start, end, ec);
    if(ec == error::need_more)
    {
        if(avail >= cfg_.header_limit)
            ec = error::header_too_large;
        return;
    }
    if(ec.failed())
        return;

    // header-fields
    it = parse_fields(it, end, ec);
    if(ec == error::need_more)
    {
        if(avail >= cfg_.header_limit)
            ec = error::header_too_large;
        return;
    }
    if(ec.failed())
        return;

    m_.header_size = it - start;
    used_ += m_.header_size;
    state_ = state::body;
}

void
basic_parser::
parse_body(
    error_code& ec)
{
    switch(state_)
    {
    case state::header:
        parse_header(ec);
        if(ec.failed())
            return;
        BOOST_ASSERT(
            state_ == state::body);
        break;
    case state::end_of_message:
        ec = error::end_of_message;
        return;
    case state::body:
        break;
    }

    auto const start = buffer_ + used_;
    auto const end = buffer_ + size_;
    auto avail = size_ - used_;

    if(m_.content_length.has_value())
    {
        // known body length
        BOOST_ASSERT(! m_.is_chunked);
        BOOST_ASSERT(
            *m_.content_length <
                cfg_.body_limit);
        if( avail > m_.remain)
            avail = static_cast<
                std::size_t>(m_.remain);
        used_ += avail;
        m_.stored += avail;
        m_.remain -= avail;
        m_.body = string_view(
            buffer_ + m_.header_size,
            m_.stored);
        if(m_.remain == 0)
        {
            state_ = state::end_of_message;
            ec = error::end_of_message;
        }
    }
}

void
basic_parser::
parse_chunk(
    error_code& ec)
{
    (void)ec;
}

//------------------------------------------------

char*
basic_parser::
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
            if(ec)
                return it;
            break;
        case field::content_length:
            do_content_length(
                p.value().value, ec);
            if(ec)
                return it;
            break;
        case field::transfer_encoding:
            do_transfer_encoding(
                p.value().value, ec);
            if(ec)
                return it;
            break;
        case field::upgrade:
            do_upgrade(
                p.value().value, ec);
            if(ec)
                return it;
            break;
        default:
            break;
        }
        cit = p.increment(
            cit, end, ec);
    }
    return it;
}

//------------------------------------------------

// https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
void
basic_parser::
do_connection(
    string_view s, error_code& ec)
{
    (void)ec;

    using namespace detail::string_literals;
    for(auto v : bnf::range<bnf::connection>(s))
    {
        if(bnf::iequals(v, "close"_sv))
        {
        }
        else if(bnf::iequals(v, "keep-alive"_sv))
        {
        }
        else if(bnf::iequals(v, "upgrade"_sv))
        {
        }
    }
}

void
basic_parser::
do_content_length(
    string_view s,
    error_code& ec)
{
    auto const start =
        s.data();
    auto const end =
        start + s.size();
    auto it = start;
    bnf::dec_number p;
    it = p.parse(
        it, end, ec);
    if(ec)
    {
        ec = error::bad_content_length;
        return;
    }
    if(it != end)
    {
        ec = error::bad_content_length;
        return;
    }
    m_.content_length = p.value();
}

void
basic_parser::
do_transfer_encoding(
    string_view s, error_code& ec)
{
    using namespace detail::string_literals;

    bnf::range<bnf::transfer_encoding> te(s);
    auto const end = te.end();
    auto it = te.begin(ec);
    if(ec)
    {
        // expected at least one encoding
        ec = error::bad_transfer_encoding;
        return;
    }
    BOOST_ASSERT(it != end);
    // handle multiple
    // Transfer-Encoding header lines
    m_.is_chunked = false;
    // get last encoding
    for(;;)
    {
        auto prev = it++;
        if(it == end)
        {
            if(bnf::iequals(
                it->name, "chunked"_sv))
                m_.is_chunked = true;
            break;
        }
    }
}

void
basic_parser::
do_upgrade(
    string_view s, error_code& ec)
{
    (void)s;
    (void)ec;
}

} // http_proto
} // boost

#endif
