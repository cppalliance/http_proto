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
enum class version : char;
#endif

struct chunk_info
{
    chunk_info() = default;

    std::uint64_t size; // of this chunk
    bnf::range<
        bnf::chunk_ext> ext; // chunk extensions (can be empty)
    bnf::range<
        bnf::header_fields> trailer;
    bool fresh;         // true if this is a fresh chunk
};

/** A parser for HTTP/1 messages.

    The parser is strict. Any malformed
    inputs according to the documented
    HTTP ABNFs is treated as an
    unrecoverable error.
*/
class parser
{
BOOST_HTTP_PROTO_PROTECTED:
    // headers have a maximum size of 2^32-1 chars
    using off_t = std::uint32_t;

    enum class state
    {
        start_line,
        header_fields,
        payload,
        end_of_message,
        end_of_stream
    };

    struct config
    {
        config() noexcept;

        std::size_t header_limit;   // max header size
        std::size_t body_limit;     // max body size
    };

    struct message
    {
        std::size_t fields;         // number of fields
        std::size_t n_header;       // bytes of header
        std::size_t n_chunk;        // bytes of chunk header
        std::size_t n_payload;      // bytes of body or chunk
        std::uint64_t n_remain;     // remaining body or chunk

        std::uint64_t payload_seen; // total body received
        std::uint64_t content_length;
        chunk_info chunk;
        http_proto::version version;// HTTP-version

        bool skip_body : 1;         // no body expected
        bool got_chunked : 1;
        bool got_close : 1;
        bool got_content_length : 1;
        bool got_keep_alive : 1;
        bool got_upgrade : 1;
        bool need_eof : 1;
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
    parser(
        context& ctx) noexcept;

public:
    BOOST_HTTP_PROTO_DECL
    ~parser();

    /** Returns true if the payload uses chunked encoding.
    */
    bool
    is_chunked() const noexcept
    {
        return m_.got_chunked;
    }

    /** Returns `true` if a complete message has been parsed.

        Calling @ref reset prepares the parser
        to process the next message in the stream.

    */
    bool
    is_end_of_message() const noexcept
    {
        return state_ ==
            state::end_of_message;
    }

    /** Returns `true` if no input remains and no more is coming.

        Calling @ref reset prepares the parser
        for additional input from a new stream.
    */
    bool
    is_end_of_stream() const noexcept
    {
        return state_ ==
            state::end_of_stream;
    }

    //http_proto::header_fields
    //fields() const noexcept;

    BOOST_HTTP_PROTO_DECL
    chunk_info
    chunk() const noexcept;

    BOOST_HTTP_PROTO_DECL
    string_view
    payload() const noexcept;

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
    discard_payload() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    discard_chunk() noexcept;

    /** Indicate that the current message has no payload.

        This informs the parser not to read a payload for
        the next message, regardless of the presence or
        absence of certain fields such as Content-Length
        or a chunked Transfer-Encoding. Depending on the
        request, some responses do not carry a body. For
        example, a 200 response to a CONNECT request from
        a tunneling proxy, or a response to a HEAD request.
        In these cases, callers may use this function inform
        the parser that no body is expected. The parser will
        consider the message complete after the header has
        been received.

        @par Preconditions

        This function must called before any calls to parse
        the current message.

        @see
            https://datatracker.ietf.org/doc/html/rfc7230#section-3.3

    */
    BOOST_HTTP_PROTO_DECL
    void
    skip_payload();

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

private:
    virtual char* parse_start_line(
        char*, char const*, error_code&) = 0;
    virtual void finish_header(error_code&) = 0;

    char* parse_fields(char*, char const*, error_code&);
    void do_connection(string_view, error_code&);
    void do_content_length(string_view, error_code&);
    void do_transfer_encoding(string_view, error_code&);
    void do_upgrade(string_view, error_code&);
};

} // http_proto
} // boost

#endif
