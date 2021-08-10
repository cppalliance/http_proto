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
    char* start,
    char const* end,
    error_code& ec)
{
    auto it = start;
    for(;;)
    {
        if(it == end)
        {
            ec = error::need_more;
            return start;
        }
        if(*it != '\r')
        {
            it = parse_field(
                it, end, ec);
            start = it;
            if(ec)
                return start;
        }
        else
        {
            ++it;
            if(it == end)
            {
                ec = error::need_more;
                return start;
            }
            if(*it != '\n')
            {
                ec = error::bad_line_ending;
                return start;
            }
            ++it;
            start = it;
            break;
        }
    }
    return start;
}

char*
basic_parser::
parse_field(
    char* const start,
    char const* end,
    error_code& ec)
{
/*
    header-field   = field-name ":" OWS field-value OWS

    field-name      = token
    field-value     = *( field-content / obs-fold )
    field-content   = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]

    obs-fold        = OWS CRLF 1*( SP / HTAB )
                    ; obsolete line folding
                    ; see Section 3.2.4

    token           = 1*tchar


    See:
    https://www.rfc-editor.org/errata/eid4189
    https://datatracker.ietf.org/doc/html/rfc7230#appendix-B
*/
    auto it = start;
    char const* k1; // end of name
    char const* v0  // start of value
        = nullptr;
    char const* v1; // end of value

    ws_set ws;
    tchar_set ts;
    field_vchar_set fvs;

    // field-name
    it = ts.skip(it, end);

    // ":"
    if(it == end)
    {
        ec = error::need_more;
        return start;
    }
    if(*it != ':')
    {
        // invalid field char
        ec = error::bad_field_name;
        return start;
    }
    if(it == start)
    {
        // missing field name
        ec = error::bad_field_name;
        return start;
    }
    k1 = it;
    ++it;

    // OWS
    it = ws.skip(it, end);

    // *( field-content / obs-fold )
    for(;;)
    {
        if(it == end)
        {
            ec = error::need_more;
            return start;
        }

        // check field-content first, as
        // it is more frequent than CRLF
        if(fvs.contains(*it))
        {
            // field-content
            if(! v0)
                v0 = it;
            ++it;
            // *field-vchar
            it = fvs.skip(it, end);
            if(it == end)
            {
                ec = error::need_more;
                return start;
            }
            v1 = it;
            continue;
        }

        // OWS
        if(ws.contains(*it))
        {
            ++it;
            it = ws.skip(it, end);
            continue;
        }

        // obs-fold / CRLF
        if(it[0] == '\r')
        {
            if(end - it < 3)
            {
                ec = error::need_more;
                return start;
            }
            if(it[1] != '\n')
            {
                // expected LF
                ec = error::bad_line_ending;
                return start;
            }
            if(! ws.contains(it[2]))
            {
                // end of line
                if(! v0)
                    v0 = it;
                v1 = it;
                it += 2;
                break;
            }
            // replace CRLF with SP SP
            it[0] = ' ';
            it[1] = ' ';
            it[2] = ' ';
            it += 3;
            // *( SP / HTAB )
            it = ws.skip(it, end);
            continue;
        }

        // illegal value
        ec = error::bad_field_value;
        return start;
    }

    string_view k(start, k1 - start);
    string_view v(v0, v1 - v0);
    auto const f =
        string_to_field(k);
    switch(f)
    {
    case field::connection:
    case field::proxy_connection:
        do_connection(v, ec);
        if(ec)
            return it;
        break;
    case field::content_length:
        do_content_length(v, ec);
        if(ec)
            return it;
        break;
    case field::transfer_encoding:
        do_transfer_encoding(v, ec);
        if(ec)
            return it;
        break;
    case field::upgrade:
        do_upgrade(v, ec);
        if(ec)
            return it;
        break;
    default:
        break;
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
    for(auto v : token_list(s))
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
    std::uint64_t n;
    it = detail::parse_u64(
        n, it, end, ec);
    if(ec)
        return;
    if(it != end)
    {
        ec = error::bad_content_length;
        return;
    }
    content_length_ = n;
}

void
basic_parser::
do_transfer_encoding(
    string_view s, error_code& ec)
{
    using namespace detail::string_literals;

    transfer_encoding_list te(s);
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
