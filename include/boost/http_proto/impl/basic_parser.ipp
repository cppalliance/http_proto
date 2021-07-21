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
    , committed_(0)
    , parsed_(0)
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

void
basic_parser::
commit(
    std::size_t n)
{
    (void)n;
}

#if 0
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
    char* p = buffer_;
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
#endif

void
basic_parser::
parse(
    error_code& ec)
{
    switch(state_)
    {
    case state::nothing_yet:
        // Parsing an empty buffer?
        BOOST_ASSERT(committed_ > 0);

        state_ = state::start_line;
        BOOST_FALLTHROUGH;

    case state::start_line:
    {
        // Nothing can come before start-line
        BOOST_ASSERT(parsed_ == 0);
        auto const parsed = 
            parse_start_line(
                buffer_,
                buffer_ + committed_,
                ec);
        parsed_ += parsed;
        if(ec)
            return;

        state_ = state::fields;
        BOOST_FALLTHROUGH;
    }

    case state::fields:
        break;
    default:
        break;
    }
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

std::pair<char*, bool>
basic_parser::
find_fast(
    char* buf,
    char const* buf_end,
    char const* ranges,
    size_t ranges_size) noexcept
{
    // VFALCO This would be a place for SIMD
    (void)buf_end;
    (void)ranges;
    (void)ranges_size;
    return {buf, false};
}

char const*
basic_parser::
find_eol(
    char const* it,
    char const* last,
    error_code& ec) noexcept
{
    for(;;)
    {
        if(it == last)
        {
            // VFALCO Do we have to clear the error?
            ec = {};
            return nullptr;
        }
        if(*it == '\r')
        {
            if(++it == last)
            {
                ec = {};
                return nullptr;
            }
            if(*it != '\n')
            {
                ec = error::bad_line_ending;
                return nullptr;
            }
            ec = {};
            return ++it;
        }
        // VFALCO We do not want to handle
        // non-conforming HTTP which uses CR
        // instead of CRLF.
        ++it;
    }
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

bool
basic_parser::
parse_version(
    char*& first,
    char const* const last,
    int& result,
    error_code& ec) noexcept
{
    auto it = first;
    if(last - it < 8)
        return false;
    if(*it++ != 'H')
    {
        ec = error::bad_version;
        return false;
    }
    if(*it++ != 'T')
    {
        ec = error::bad_version;
        return false;
    }
    if(*it++ != 'T')
    {
        ec = error::bad_version;
        return false;
    }
    if(*it++ != 'P')
    {
        ec = error::bad_version;
        return false;
    }
    if(*it++ != '/')
    {
        ec = error::bad_version;
        return false;
    }
    if(! is_digit(*it))
    {
        ec = error::bad_version;
        return false;
    }
    result = 10 * (*it++ - '0');
    if(*it++ != '.')
    {
        ec = error::bad_version;
        return false;
    }
    if(! is_digit(*it))
    {
        ec = error::bad_version;
        return false;
    }
    result += *it++ - '0';
    if(result != 10 && result != 11)
    {
        ec = error::bad_version;
        return false;
    }
    first = it;
    return true;
}

void
basic_parser::
parse_fields(
    char*& in,
    char const* last,
    error_code& ec)
{
    // https://stackoverflow.com/questions/686217/maximum-on-http-header-values
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
        parse_field(p, last, ec);
        if(ec)
            return;
        in = p;
    }
}

void
basic_parser::
parse_field(
    char*& p,
    char const* last,
    error_code& ec)
{
/*  header-field    = field-name ":" OWS field-value OWS

    field-name      = token
    field-value     = *( field-content / obs-fold )
    field-content   = field-vchar [ 1*( SP / HTAB ) field-vchar ]
    field-vchar     = VCHAR / obs-text

    obs-fold        = CRLF 1*( SP / HTAB )
                    ; obsolete line folding
                    ; see Section 3.2.4

    token           = 1*<any CHAR except CTLs or separators>
    CHAR            = <any US-ASCII character (octets 0 - 127)>
    sep             = "(" | ")" | "<" | ">" | "@"
                    | "," | ";" | ":" | "\" | <">
                    | "/" | "[" | "]" | "?" | "="
                    | "{" | "}" | SP | HT
*/
    static char const* is_token =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\1\0\1\1\1\1\1\0\0\1\1\0\1\1\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
        "\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\1"
        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    // name
    BOOST_ALIGNMENT(16) static const char ranges1[] =
        "\x00 "  /* control chars and up to SP */
        "\"\""   /* 0x22 */
        "()"     /* 0x28,0x29 */
        ",,"     /* 0x2c */
        "//"     /* 0x2f */
        ":@"     /* 0x3a-0x40 */
        "[]"     /* 0x5b-0x5d */
        "{\377"; /* 0x7b-0xff */
    auto first = p;
    bool found;
    std::tie(p, found) = find_fast(
        p, last, ranges1, sizeof(ranges1)-1);
    if(! found && p >= last)
    {
        ec = error::need_more;
        return;
    }
    for(;;)
    {
        if(*p == ':')
            break;
        if(! is_token[static_cast<
            unsigned char>(*p)])
        {
            ec = error::bad_field;
            return;
        }
        ++p;
        if(p >= last)
        {
            ec = error::need_more;
            return;
        }
    }
    if(p == first)
    {
        // empty name
        ec = error::bad_field;
        return;
    }
#if 0
    name = make_string(first, p);
    ++p; // eat ':'
    char const* token_last = nullptr;
    for(;;)
    {
        // eat leading ' ' and '\t'
        for(;;++p)
        {
            if(p + 1 > last)
            {
                ec = error::need_more;
                return;
            }
            if(! (*p == ' ' || *p == '\t'))
                break;
        }
        // parse to CRLF
        first = p;
        p = parse_token_to_eol(p, last, token_last, ec);
        if(ec)
            return;
        if(! p)
        {
            ec = error::bad_value;
            return;
        }
        // Look 1 char past the CRLF to handle obs-fold.
        if(p + 1 > last)
        {
            ec = error::need_more;
            return;
        }
        token_last =
            trim_back(token_last, first);
        if(*p != ' ' && *p != '\t')
        {
            value = make_string(first, token_last);
            return;
        }
        ++p;
        if(token_last != first)
            break;
    }
    buf.clear();
    if (!buf.try_append(first, token_last))
    {
        ec = error::header_limit;
        return;
    }

    BOOST_ASSERT(! buf.empty());
    for(;;)
    {
        // eat leading ' ' and '\t'
        for(;;++p)
        {
            if(p + 1 > last)
            {
                ec = error::need_more;
                return;
            }
            if(! (*p == ' ' || *p == '\t'))
                break;
        }
        // parse to CRLF
        first = p;
        p = parse_token_to_eol(p, last, token_last, ec);
        if(ec)
            return;
        if(! p)
        {
            ec = error::bad_value;
            return;
        }
        // Look 1 char past the CRLF to handle obs-fold.
        if(p + 1 > last)
        {
            ec = error::need_more;
            return;
        }
        token_last = trim_back(token_last, first);
        if(first != token_last)
        {
            if (!buf.try_push_back(' ') ||
                !buf.try_append(first, token_last))
            {
                ec = error::header_limit;
                return;
            }
        }
        if(*p != ' ' && *p != '\t')
        {
            value = {buf.data(), buf.size()};
            return;
        }
        ++p;
    }
#endif
}

} // http_proto
} // boost

#endif
