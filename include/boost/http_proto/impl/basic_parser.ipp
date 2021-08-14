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
{
}

constexpr
basic_parser::
message::
message() noexcept
    : header_size(0)
    , body_size(boost::none)
    , body_remain(0)
    , version(0)
    , is_chunked(false)
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
}

void
basic_parser::
discard_body() noexcept
{
}

void
basic_parser::
discard_chunk() noexcept
{
}

//------------------------------------------------

void
basic_parser::
do_parse(
    error_code& ec)
{
    auto const start =
        buffer_ + used_;
    auto const end =
        buffer_ + size_;
    auto avail = static_cast<
        std::size_t>(end - start);

    switch(state_)
    {
    case state::start_line:
    {
        break;
    }
    case state::fields:
    {
        break;
    }
    case state::body:
    {
        if(m_.body_size.has_value())
        {
            // known body length
            if( avail > *m_.body_size)
                avail = static_cast<
                    std::size_t>(
                        *m_.body_size);
#if 0
            m_.body = string_view(
                buffer_ + m_.header_size,
                buffer_ + m_.header_size +
#endif
        }
        else
        {
            // unknown body length
        }
        break;
    }
    case state::chunk:
    {
        bnf::chunk_part p;
        auto it = p.parse(
            start, end, ec);
        if(ec)
            return;
        break;
    }
    default:
    {
        break;
    }
    }
}

void
basic_parser::
parse_header(
    error_code& ec)
{
    ec = {};
    api_ = api::header;

    auto start = buffer_ + used_;
    char const* const end = [this]
    {
        if(size_ <= cfg_.header_limit)
            return buffer_ + size_;
        return buffer_ + cfg_.header_limit;
    }();

    switch(state_)
    {
    case state::nothing_yet:
    {
        state_ = state::start_line;
        BOOST_FALLTHROUGH;
    }
    case state::start_line:
    {
        // Nothing can come before start-line
        BOOST_ASSERT(used_ == 0);
        start = parse_start_line(
            start, end, ec);
        if(ec)
        {
            if( ec == error::need_more &&
                size_ >= cfg_.header_limit)
                ec = error::header_too_large;
            goto finish;
        }
        state_ = state::fields;
        BOOST_FALLTHROUGH;
    }
    case state::fields:
    {
        start = parse_fields(
            start, end, ec);
        if(ec)
        {
            if( ec == error::need_more &&
                size_ >= cfg_.header_limit)
                ec = error::header_too_large;
            goto finish;
        }
        m_.header_size =
            start - buffer_;
        state_ = state::body;
        break;
    }
    default:
        break;
    }

finish:
    used_ = start - buffer_;
}

void
basic_parser::
parse_body(
    error_code& ec)
{
    ec = {};
    api_ = api::body;

    switch(state_)
    {
    case state::nothing_yet:
    case state::start_line:
    case state::fields:
    {
        parse_header(ec);
        if(ec.failed())
            return;
        state_ = state::body;
        BOOST_FALLTHROUGH;
    }
    case state::body:
    {
        break;
    }

    default:
        break;
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
    m_.body_size = p.value();
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
