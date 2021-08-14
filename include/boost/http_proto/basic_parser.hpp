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
#include <boost/http_proto/trivial_optional.hpp>
#include <boost/http_proto/bnf/chunk_ext.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
#endif

struct chunk_info
{
    constexpr chunk_info() = default;

    std::uint64_t size; // of this chunk
    bnf::chunk_ext ext; // chunk extensions (can be empty)
    bnf::range<
        bnf::header_fields> trailer;
    bool fresh;         // true if this is a fresh chunk
    bool last;          // last chunk
};

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
        header,
        body,
        end_of_message
    };

    struct config
    {
        constexpr config() noexcept;

        std::size_t header_limit;   // max header size
        std::size_t body_limit;     // max body size
    };

    struct message
    {
        std::size_t header_size;
        string_view body;           // body part
        std::size_t stored;         // body stored
        std::uint64_t remain;       // body remaining
        trivial_optional<
            std::uint64_t> content_length;
        char version;               // HTTP-version, 0 or 1
        chunk_info chunk;

        bool is_chunked : 1;
    };

    context& ctx_;
    char* buffer_;
    std::size_t cap_;           // allocated size
    std::size_t size_;          // committed part
    std::size_t used_;          // parsed part
    state state_;

    bool got_eof_;

    config cfg_;
    message m_;

    explicit
    basic_parser(
        context& ctx) noexcept;

public:
    BOOST_HTTP_PROTO_DECL
    ~basic_parser();

    /** Returns true if the payload uses chunked encoding.
    */
    bool
    is_chunked() const noexcept
    {
        return m_.is_chunked;
    }

    /** Returns `true` if a complete message has been parsed.
    */
    bool
    is_end_of_message() const noexcept
    {
        return state_ ==
            state::end_of_message;
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
    discard_header() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    discard_body() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    discard_chunk() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    parse_header(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_body(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    parse_chunk(
        error_code& ec);

    string_view
    body() const noexcept
    {
        return m_.body;
    }

    chunk_info
    chunk() const noexcept
    {
        return m_.chunk;
    }

private:
    friend class request_parser;
    friend class response_parser;

    virtual char* parse_start_line(
        char*, char const*, error_code&) = 0;
    virtual void finish_header(error_code&) = 0;

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
