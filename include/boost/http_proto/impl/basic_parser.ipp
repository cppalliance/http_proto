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
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/detail/sv.hpp>
#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>
#include <boost/http_proto/bnf/token_list.hpp>
#include <boost/http_proto/bnf/transfer_encoding_list.hpp>
#include <boost/assert.hpp>
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------

basic_parser::
basic_parser(
    context& ctx) noexcept
    : ctx_(ctx)
    , state_(state::nothing_yet)
    , buffer_(nullptr)
    , cap_(0)
    , size_(0)
    , used_(0)
    , header_limit_(8192)
{
    std::memset(&f_,
        0, sizeof(f_));
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
    header_size_ = 0;
    state_ = state::nothing_yet;

    std::memset(&f_,
        0, sizeof(f_));
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

//------------------------------------------------

void
basic_parser::
parse_header(
    error_code& ec)
{
    ec = {};

    auto start = buffer_ + used_;
    char const* const end = [this]
    {
        if(size_ <= header_limit_)
            return buffer_ + size_;
        return buffer_ + header_limit_;
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
                size_ >= header_limit_)
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
                size_ >= header_limit_)
                ec = error::header_too_large;
            goto finish;
        }
        header_size_ =
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
    ec = {}; // redundant?

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
parse_body_part(
    error_code& ec)
{
    (void)ec;
    switch(state_)
    {
    case state::nothing_yet:
        state_ = state::start_line;
        BOOST_FALLTHROUGH;
    }
}

void
basic_parser::
parse_chunk_ext(
    error_code& ec)
{
    (void)ec;
}

void
basic_parser::
parse_chunk_part(
    error_code& ec)
{
    (void)ec;
}

void
basic_parser::
parse_chunk_trailer(
    error_code& ec)
{
    (void)ec;
}

string_view
basic_parser::
body() const
{
    return {nullptr, 0};
}

//------------------------------------------------

char*
basic_parser::
parse_fields(
    char* const start,
    char const* const end,
    error_code& ec)
{
    bnf::header_fields_bnf p;
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
        if(p.value.has_obs_fold)
            bnf::replace_obs_fold(it, cit);
        it = start + (cit - start);
        auto const f =
            string_to_field(
                p.value.name);
        switch(f)
        {
        case field::connection:
        case field::proxy_connection:
            do_connection(
                p.value.value, ec);
            if(ec)
                return it;
            break;
        case field::content_length:
            do_content_length(
                p.value.value, ec);
            if(ec)
                return it;
            break;
        case field::transfer_encoding:
            do_transfer_encoding(
                p.value.value, ec);
            if(ec)
                return it;
            break;
        case field::upgrade:
            do_upgrade(
                p.value.value, ec);
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
    for(auto v : bnf::token_list(s))
    {
        if(iequals(v, "close"_sv))
        {
        }
        else if(iequals(v, "keep-alive"_sv))
        {
        }
        else if(iequals(v, "upgrade"_sv))
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
    bnf::number_bnf p;
    it = p.parse_element(
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
    content_length_ = p.value;
}

void
basic_parser::
do_transfer_encoding(
    string_view s, error_code& ec)
{
    using namespace detail::string_literals;

    bnf::transfer_encoding_list te(s);
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
    f_.chunked = false;
    // get last encoding
    for(;;)
    {
        auto prev = it++;
        if(it == end)
        {
            if(iequals(
                it->name, "chunked"_sv))
                f_.chunked = true;
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
