//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BASIC_PARSER_HPP
#define BOOST_HTTP_PROTO_BASIC_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace boost {
namespace http_proto {

class basic_parser
{
protected:
    // headers have a maximum size of 65536 chars
    using off_t = std::uint16_t;

private:
    enum class state
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
protected:
    char* buffer_;
private:
    std::size_t capacity_;          // allocated size
    std::size_t committed_;         // committed part
protected:
    std::size_t parsed_;            // parsed part
private:

    state state_;
    std::uint32_t header_limit_;    // max header size
    std::size_t skip_;              // offset to continue parse



protected:
    static unsigned constexpr flagSkipBody              = 1<<  0;
    static unsigned constexpr flagEager                 = 1<<  1;
    static unsigned constexpr flagGotSome               = 1<<  2;
    static unsigned constexpr flagHasBody               = 1<<  3;
    static unsigned constexpr flagHTTP11                = 1<<  4;
    static unsigned constexpr flagNeedEOF               = 1<<  5;
    static unsigned constexpr flagExpectCRLF            = 1<<  6;
    static unsigned constexpr flagConnectionClose       = 1<<  7;
    static unsigned constexpr flagConnectionUpgrade     = 1<<  8;
    static unsigned constexpr flagConnectionKeepAlive   = 1<<  9;
    static unsigned constexpr flagContentLength         = 1<< 10;
    static unsigned constexpr flagChunked               = 1<< 11;
    static unsigned constexpr flagUpgrade               = 1<< 12;
    static unsigned constexpr flagFinalChunk            = 1<< 13;

    unsigned f_;                    // flags

    static bool is_digit(char) noexcept;
    static bool is_print(char) noexcept;

public:
    BOOST_HTTP_PROTO_DECL
    ~basic_parser();

    BOOST_HTTP_PROTO_DECL
    basic_parser() noexcept;

    /** Returns `true` if more input data is required.
    */
    bool
    need_more() const noexcept;

    /** Returns `true` if a complete message has been parsed.
    */
    bool
    is_done() const noexcept
    {
        return false;
    }

    /** Prepare the parser for a new message.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset();

    BOOST_HTTP_PROTO_DECL
    std::pair<void*, std::size_t>
    prepare();

    BOOST_HTTP_PROTO_DECL
    void
    commit(std::size_t n);

    //void commit_eof(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse(error_code& ec);

protected:
    virtual
    bool
    parse_start_line(
        char*& first,
        char const* last,
        error_code& ec) = 0;

    virtual
    void
    finish_header(
        error_code& ec) = 0;

private:
    void
    maybe_need_more(
        char const* p,
        std::size_t n,
        error_code& ec) noexcept;

    static
    std::pair<char*, bool>
    find_fast(
        char* buf,
        char const* buf_end,
        char const* ranges,
        size_t ranges_size) noexcept;

    static
    char const*
    find_eol(
        char const* it,
        char const* last,
        error_code& ec) noexcept;

    static
    char const*
    find_eom(
        char const* p,
        char const* last) noexcept;

    void
    parse_fields(
        char*& p,
        char const* last,
        error_code& ec);

protected:
    static
    bool
    parse_version(
        char*& it,
        char const* last,
        int& result,
        error_code& ec) noexcept;

    static
    void
    parse_field(
        char*& p,
        char const* last,
        error_code& ec);
};

} // http_proto
} // boost

#endif
