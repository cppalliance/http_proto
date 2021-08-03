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
#include <boost/assert.hpp>
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------

bool
basic_parser::
is_digit(char c) noexcept
{
    return static_cast<
        unsigned char>(c-'0') < 10;
}

bool
basic_parser::
is_print(char c) noexcept
{
    return static_cast<
        unsigned char>(c-32) < 95;
}

//------------------------------------------------

basic_parser::
basic_parser(
    context& ctx) noexcept
    : ctx_(ctx)
    , buffer_(nullptr)
    , capacity_(0)
    , committed_(0)
    , parsed_(0)
    , state_(state::nothing_yet)
    , header_limit_(8192)
    , skip_(0)
    , f_(0)
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
}

std::pair<void*, std::size_t>
basic_parser::
prepare()
{
    if(! buffer_)
    {
        buffer_ = new char[4096];
        capacity_ = 4096;
    }
    else if(capacity_ <= committed_)
    {
        auto buffer = new char[
            capacity_ + 4096];
        std::memcpy(buffer,
            buffer_, committed_);
        delete[] buffer_;
        buffer_ = buffer;
        capacity_ = capacity_ + 4096;
    }

    return {
        buffer_ + committed_,
        capacity_ - committed_ };
}

void
basic_parser::
commit(
    std::size_t n)
{
    // VFALCO Not sure about this
    BOOST_ASSERT(n > 0);

    BOOST_ASSERT(n <=
        capacity_ - committed_);
    committed_ += n;
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

    // parse algorithms assume at
    // least one committed char.
    if(committed_ == 0)
        return;

    auto first = buffer_ + parsed_;
    char const* const last =
        buffer_ + committed_;

    switch(state_)
    {
    case state::nothing_yet:
        state_ = state::start_line;
        BOOST_FALLTHROUGH;

    case state::start_line:
    {
        // Nothing can come before start-line
        BOOST_ASSERT(parsed_ == 0);
        parse_start_line(first, last, ec);
        if(ec)
            return;
        state_ = state::fields;
        BOOST_FALLTHROUGH;
    }

    case state::fields:
    {
        if(! parse_fields(
                first, last, ec))
            break;
        BOOST_ASSERT(! ec);
        state_ = state::body;
        break;
    }

    default:
        break;
    }

    parsed_ = first - buffer_;
}

void
basic_parser::
parse_body(
    error_code& ec)
{
    switch(state_)
    {
    case state::nothing_yet:
        state_ = state::start_line;
        BOOST_FALLTHROUGH;
    }
}

void
basic_parser::
parse_body_part(
    error_code& ec)
{
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
}

void
basic_parser::
parse_chunk_part(
    error_code& ec)
{
}

void
basic_parser::
parse_chunk_trailer(
    error_code& ec)
{
}

string_view
basic_parser::
body() const
{
    return {nullptr, 0};
}

void
basic_parser::
consume_header() noexcept
{
}

void
basic_parser::
consume_body() noexcept
{
}

//------------------------------------------------

#if 0
bool
basic_parser::
parse_u64(
    string_view s,
    std::uint64_t& v)
{
    char const* it = s.data();
    char const* last = it + s.size();
    if(it == last)
        return false;
    std::uint64_t tmp = 0;
    do
    {
        if((! is_digit(*it)) ||
            tmp > (std::numeric_limits<std::uint64_t>::max)() / 10)
            return false;
        tmp *= 10;
        std::uint64_t const d = *it - '0';
        if((std::numeric_limits<std::uint64_t>::max)() - tmp < d)
            return false;
        tmp += d;
    }
    while(++it != last);
    v = tmp;
    return true;
}
#endif

void
basic_parser::
parse_version(
    char*& first,
    char const* const last,
    error_code& ec) noexcept
{
    auto const need_more =
        [&ec]{ ec = error::need_more; };
    auto it = first;
    if(last - it < 8)
        return need_more();
    if(std::memcmp(it, "HTTP/1.", 7) != 0)
    {
        ec = error::bad_version;
        return;
    }
    it += 7;
    if(*it == 0)
    {
        version_ = 0;
    }
    else if(*it == 1)
    {
        version_ = 1;
    }
    else
    {
        ec = error::bad_version;
        return;
    }
    ++it;
    first = it;
}

bool
basic_parser::
parse_fields(
    char*& first,
    char const* last,
    error_code& ec)
{
    auto in = first;
    for(;;)
    {
        if(last - in < 2)
            return false;
        if(in[0] == '\r')
        {
            if(in[1] != '\n')
            {
                ec = error::bad_line_ending;
                return false;
            }
            // end of header
            in = in + 2;
            first = in;
            break;
        }
        if(! parse_field(in, last, ec))
            return false;
        BOOST_ASSERT(! ec);
        first = in;
    }
    return true;
}

bool
basic_parser::
parse_field(
    char*& first,
    char const* last,
    error_code& ec)
{
/*
    header-field   = field-name ":" OWS field-value OWS

    field-name      = token
    field-value     = *( field-content / obs-fold )
    field-content   = field-vchar [ 1*( SP / HTAB ) field-vchar ]
    field-vchar     = VCHAR / obs-text

    VCHAR           =  %x21-7E
                    ; visible (printing) characters

    obs-fold        = CRLF 1*( SP / HTAB )
                    ; obsolete line folding
                    ; see Section 3.2.4

    obs-text        = %x80-FF

    token           = 1*tchar
    tchar           = "!" / "#" / "$" / "%" / "&" / "'" /
                      "*" / "+" / "-" / "." / "^" / "_" /
                      "`" / "|" / "~" / DIGIT / ALPHA
*/
    static char const* const is_tchar =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\1\0\1\1\1\1\1\0\0\1\1\0\1\1\0" "\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
        "\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    static char const* const is_vchar =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1" "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1";

    auto in = first;

    // field-name
    for(;;)
    {
        if(*in == ':')
            break;
        if(! is_tchar[static_cast<
            unsigned char>(*in)])
        {
            ec = error::bad_field;
            return false;
        }
        ++in;
        if(last - in < 3)
        {
            // need at least 3 chars
            // to detect CRLF SP.
            return false;
        }
    }
    if(in == first)
    {
        // empty name
        ec = error::bad_field;
        return false;
    }
    //off_t n_name = in - first;
    ++in; // eat ':'

    // consume OWS and obs-fold
    for(;;)
    {
        if(last - in < 3)
        {
            // need at least 3 chars
            // to detect CRLF SP.
            return false;
        }
        if( *in == ' ' ||
            *in == '\t')
        {
            // OWS
            ++in;
            continue;
        }
        if(*in == '\r')
        {
            if(in[1] != '\n')
            {
                ec = error::bad_line_ending;
                return false;
            }
            if( in[2] != ' ' &&
                in[2] != '\t')
            {
                // empty field-value
                in += 2;
                goto done;
            }
            // replace obs-fold with SP
            *in++ = ' ';
            *in++ = ' ';
            *in++ = ' ';
            continue;
        }
        // field-value
        break;
    }

    for(;;)
    {
        if(last - in < 3)
        {
            // need at least 3 chars
            // to detect CRLF SP.
            return false;
        }
        if( is_vchar[static_cast<
                unsigned char>(*in)] ||
            *in == ' ' ||
            *in == '\t')
        {
            ++in;
            continue;
        }
        if(*in != '\r')
        {
            // bad field-char
            ec = error::bad_value;
            return false;
        }
        if(in[1] != '\n')
        {
            ec = error::bad_line_ending;
            return false;
        }
        if( in[2] != ' ' &&
            in[2] != '\t')
        {
            // end of field
            in += 2;
            goto done;
        }
        // replace obs-fold with SP
        *in++ = ' ';
        *in++ = ' ';
        *in++ = ' ';
    }

done:
    first = in;
    return true;
}

} // http_proto
} // boost

#endif
