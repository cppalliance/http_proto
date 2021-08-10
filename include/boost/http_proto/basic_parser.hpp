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
#include <boost/http_proto/string_view.hpp>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
#endif

/** A parser for HTTP/1 messages.

    The parser is strict. Any malformed
    inputs according to the documented
    HTTP ABNFs is treated as an
    unrecoverable error.
*/
class basic_parser
{
private:
    // headers have a maximum size of 65536 chars
    using off_t = std::uint16_t;

    enum class state
    {
        nothing_yet = 0,
        start_line,
        fields,
        body,
#if 0
        body_to_eof0,
        body_to_eof,
        chunk_header0,
        chunk_header,
        chunk_body,
#endif
        complete
    };

    context& ctx_;
    char* buffer_;
    std::size_t cap_;           // allocated size
    std::size_t size_;          // committed part
    std::size_t used_;          // parsed part
    std::size_t header_size_;   // full header size

    std::uint64_t
        content_length_;
    std::uint64_t body_size_;

    state state_;

    std::size_t header_limit_;  // max header size

    bool flag_chunked_;

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

    char version_;                  // HTTP-version, 0 or 1

    explicit
    basic_parser(
        context& ctx) noexcept;

public:
    BOOST_HTTP_PROTO_DECL
    ~basic_parser();

    /** Returns `true` if a complete message has been parsed.
    */
    bool
    is_done() const noexcept
    {
        return state_ == state::complete;
    }

    /** Prepare the parser for the next message.

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

    BOOST_HTTP_PROTO_DECL
    void
    commit_eof();

    BOOST_HTTP_PROTO_DECL
    void
    parse_header(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_body(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_body_part(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_chunk_ext(
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_chunk_part(
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_chunk_trailer(
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    string_view
    body() const;

private:
    friend class request_parser;
    friend class response_parser;

    virtual char* parse_start_line(
        char*, char const*, error_code&) = 0;
    virtual void finish_header(error_code&) = 0;

    char* parse_version(char*, char const*, error_code&);
    char* parse_fields(char*, char const*, error_code&);
    char* parse_field(char*, char const*, error_code&);
    void do_connection(string_view, error_code&);
    void do_content_length(string_view, error_code&);
    void do_transfer_encoding(string_view, error_code&);
    void do_upgrade(string_view, error_code&);
};

} // http_proto
} // boost

#endif
