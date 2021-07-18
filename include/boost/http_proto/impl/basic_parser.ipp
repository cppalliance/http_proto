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

namespace boost {
namespace http_proto {

//------------------------------------------------

enum class basic_parser::state
{
    nothing_yet = 0,
    start_line,
    fields,
    body0,
    body,
    body_to_eof0,
    body_to_eof,
    chunk_header0,
    chunk_header,
    chunk_body,
    complete
};

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
~basic_parser()
{
    if(buffer_)
        delete[] buffer_;
}

basic_parser::
basic_parser() noexcept
    : buffer_(nullptr)
    , capacity_(0)
    , size_(0)
    , state_(state::nothing_yet)
    , header_limit_(8192)
    , skip_(0)
    , f_(0)
{
}

bool
basic_parser::
need_more() const noexcept
{
    return true;
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

    return { buffer_, capacity_ };
}

std::size_t
basic_parser::
commit(
    std::size_t n,
    error_code& ec)
{
#if 0
    // If this goes off you have tried to parse more data after the parser
    // has completed. A common cause of this is re-using a parser, which is
    // not supported. If you need to re-use a parser, consider storing it
    // in an optional. Then reset() and emplace() prior to parsing each new
    // message.
    BOOST_ASSERT(!is_done());
    if (is_done())
    {
        ec = error::stale_parser;
        return 0;
    }
    auto p = static_cast<char const*>(buffer.data());
#endif
    char const* p = buffer_;
    auto const p0 = p;
    auto const p1 = p0 + n;
    ec = {};

loop:
    switch(state_)
    {
    case state::nothing_yet:
        if(n == 0)
        {
            ec = error::need_more;
            return 0;
        }
        state_ = state::start_line;
        BOOST_FALLTHROUGH;

    case state::start_line:
    {
        maybe_need_more(p, n, ec);
        if(ec)
            goto done;
        this->parse_start_line(p,
            p + (std::min<std::size_t>)(
            header_limit_, n), ec);
        if(ec)
        {
            if(ec == error::need_more)
            {
                if(n >= header_limit_)
                {
                    ec = error::header_limit;
                    goto done;
                }
                if(p + 3 <= p1)
                    skip_ = static_cast<
                        std::size_t>(p1 - p - 3);
            }
            goto done;
        }
        state_ = state::fields;
        BOOST_ASSERT(! is_done());
        n = static_cast<std::size_t>(p1 - p);
        if(p >= p1)
        {
            ec = error::need_more;
            goto done;
        }
        BOOST_FALLTHROUGH;
    }

    case state::fields:
        maybe_need_more(p, n, ec);
        if(ec)
            goto done;
        parse_fields(p,
            p + (std::min<std::size_t>)(
            header_limit_, n), ec);
        if(ec)
        {
            if(ec == error::need_more)
            {
                if(n >= header_limit_)
                {
                    ec = error::header_limit;
                    goto done;
                }
                if(p + 3 <= p1)
                    skip_ = static_cast<
                        std::size_t>(p1 - p - 3);
            }
            goto done;
        }
        finish_header(ec);
        if(ec)
            goto done;
        break;

#if 0
    case state::body0:
        BOOST_ASSERT(! skip_);
        this->on_body_init_impl(content_length(), ec);
        if(ec)
            goto done;
        state_ = state::body;
        BOOST_FALLTHROUGH;

    case state::body:
        BOOST_ASSERT(! skip_);
        parse_body(p, n, ec);
        if(ec)
            goto done;
        break;

    case state::body_to_eof0:
        BOOST_ASSERT(! skip_);
        this->on_body_init_impl(content_length(), ec);
        if(ec)
            goto done;
        state_ = state::body_to_eof;
        BOOST_FALLTHROUGH;

    case state::body_to_eof:
        BOOST_ASSERT(! skip_);
        parse_body_to_eof(p, n, ec);
        if(ec)
            goto done;
        break;

    case state::chunk_header0:
        this->on_body_init_impl(content_length(), ec);
        if(ec)
            goto done;
        state_ = state::chunk_header;
        BOOST_FALLTHROUGH;

    case state::chunk_header:
        parse_chunk_header(p, n, ec);
        if(ec)
            goto done;
        break;

    case state::chunk_body:
        parse_chunk_body(p, n, ec);
        if(ec)
            goto done;
        break;
#endif

    case state::complete:
        ec = {};
        goto done;
    }
    // VFALCO FIX
#if 0
    if(p < p1 && ! is_done() && eager())
    {
        n = static_cast<std::size_t>(p1 - p);
        goto loop;
    }
#endif
done:
    return static_cast<std::size_t>(p - p0);
}

//------------------------------------------------

void
basic_parser::
maybe_need_more(
    char const* p,
    std::size_t n,
    error_code& ec) noexcept
{
    if(skip_ == 0)
        return;
    if( n > header_limit_)
        n = header_limit_;
    if(n < skip_ + 4)
    {
        ec = error::need_more;
        return;
    }
    auto const term =
        find_eom(p + skip_, p + n);
    if(! term)
    {
        skip_ = n - 3;
        if(skip_ + 4 > header_limit_)
        {
            ec = error::header_limit;
            return;
        }
        ec = error::need_more;
        return;
    }
    skip_ = 0;
}

char const*
basic_parser::
find_eom(
    char const* p,
    char const* last) noexcept
{
    for(;;)
    {
        if(p + 4 > last)
            return nullptr;
        if(p[3] != '\n')
        {
            if(p[3] == '\r')
                ++p;
            else
                p += 4;
        }
        else if(p[2] != '\r')
        {
            p += 4;
        }
        else if(p[1] != '\n')
        {
            p += 2;
        }
        else if(p[0] != '\r')
        {
            p += 2;
        }
        else
        {
            return p + 4;
        }
    }
}

void
basic_parser::
parse_version(
    char const*& it, char const* last,
    int& result, error_code& ec) noexcept
{
    if(last - it < 8) // it + 8 > last
    {
        ec = error::need_more;
        return;
    }
    if(*it++ != 'H')
    {
        ec = error::bad_version;
        return;
    }
    if(*it++ != 'T')
    {
        ec = error::bad_version;
        return;
    }
    if(*it++ != 'T')
    {
        ec = error::bad_version;
        return;
    }
    if(*it++ != 'P')
    {
        ec = error::bad_version;
        return;
    }
    if(*it++ != '/')
    {
        ec = error::bad_version;
        return;
    }
    if(! is_digit(*it))
    {
        ec = error::bad_version;
        return;
    }
    result = 10 * (*it++ - '0');
    if(*it++ != '.')
    {
        ec = error::bad_version;
        return;
    }
    if(! is_digit(*it))
    {
        ec = error::bad_version;
        return;
    }
    result += *it++ - '0';
}

void
basic_parser::
parse_fields(
    char const*& in,
    char const* last,
    error_code& ec)
{
#if 0
    string_view name;
    string_view value;
    // https://stackoverflow.com/questions/686217/maximum-on-http-header-values
    beast::detail::char_buffer<max_obs_fold> buf;
    auto p = in;
    for(;;)
    {
        if(p + 2 > last)
        {
            ec = error::need_more;
            return;
        }
        if(p[0] == '\r')
        {
            if(p[1] != '\n')
                ec = error::bad_line_ending;
            in = p + 2;
            return;
        }
        parse_field(p, last, name, value, buf, ec);
        if(ec)
            return;
        auto const f = string_to_field(name);
        do_field(f, value, ec);
        if(ec)
            return;
        this->on_field_impl(f, name, value, ec);
        if(ec)
            return;
        in = p;
    }
#endif
}

} // http_proto
} // boost

#endif
