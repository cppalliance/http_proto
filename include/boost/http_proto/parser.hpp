//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BASIC_PARSER_HPP
#define BOOST_HTTP_PROTO_BASIC_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/sink.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/circular_buffer.hpp>
#include <boost/http_proto/detail/flat_buffer.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/optional.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
enum class version : char;
class request_parser;
class response_parser;
struct brotli_decoder_t;
struct brotli_encoder_t;
struct deflate_decoder_t;
struct deflate_encoder_t;
struct gzip_decoder_t;
struct gzip_encoder_t;
namespace detail {
struct codec;
}
#endif

/** A parser for HTTP/1 messages.

    The parser is strict. Any malformed
    inputs according to the documented
    HTTP ABNFs is treated as an
    unrecoverable error.
*/
class BOOST_SYMBOL_VISIBLE
    parser
{
public:
    /** Parser configuration settings

        @see
            @li <a href="https://stackoverflow.com/questions/686217/maximum-on-http-header-values"
                >Maximum on HTTP header values (Stackoverflow)</a>
    */
    struct config_base
    {
        /** Largest allowed size for the headers.

            This determines an upper bound on the
            allowed size of the start-line plus
            all of the individual fields in the
            headers. This counts all delimiters
            including trailing CRLFs.
        */
        std::size_t headers_limit = 16 * 1024;

        /** Largest allowed size for the start-line.

            This determines an upper bound on the
            allowed size for the request-line of
            an HTTP request or the status-line of
            an HTTP response.
        */
        std::size_t start_line_limit = 4096;

        /** Largest size for one field.

            This determines an upper bound on the
            allowed size for any single header
            in an HTTP message. This counts
            the field name, field value, and
            delimiters including a trailing CRLF.
        */
        std::size_t field_size_limit = 4096;

        /** Largest allowed number of fields.

            This determines an upper bound on the
            largest number of individual header
            fields that may appear in an HTTP
            message.
        */
        std::size_t fields_limit = 100;

        /** Largest allowed size for a content body.

            The size of the body is measured
            after removing any transfer encodings,
            including a chunked encoding.
        */
        std::uint64_t body_limit = 64 * 1024;
    };

    using buffers = mutable_buffers_pair;

private:
    BOOST_HTTP_PROTO_DECL parser(
        detail::kind, config_base const&);
    BOOST_HTTP_PROTO_DECL void construct(
        std::size_t extra_buffer_size);
public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~parser();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    parser(parser&&) noexcept;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

#if 0
    /** Return true if any input was committed.
    */
    bool
    got_some() const noexcept
    {
        return st_ != state::need_start;
    }
#endif

    /** Return true if the complete header was parsed.
    */
    bool
    got_header() const noexcept
    {
        return st_ > state::headers;
    }

    /** Returns `true` if a complete message has been parsed.

        Calling @ref reset prepares the parser
        to process the next message in the stream.

    */
    bool
    is_complete() const noexcept
    {
        return st_ == state::complete;
    }

    BOOST_HTTP_PROTO_DECL
    string_view
    body() const noexcept;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Prepare for a new stream.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset() noexcept;

private:
    // New message on the current stream
    BOOST_HTTP_PROTO_DECL void
        start_impl(bool head_response);
public:

    /** Return the input buffer
    */
    BOOST_HTTP_PROTO_DECL
    buffers
    prepare();

    /** Commit bytes to the input buffer
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(
        std::size_t n);

    /** Indicate there will be no more input
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit_eof();

    /** Parse pending input data
    */
    BOOST_HTTP_PROTO_DECL
    void
    parse(
        error_code& ec);

    //--------------------------------------------

    /** Return any leftover data

        This is used to forward unconsumed data
        that could lie past the last message.
        For example on a CONNECT request there
        could be additional protocol-dependent
        data that we want to retrieve.
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    release_buffered_data() noexcept;

private:
    void apply_params() noexcept
    {
    }

    void apply_param(...) = delete;

    template<class P0, class... Pn>
    void
    apply_params(P0&& p0, Pn&&... pn)
    {
        // If you get an error here it means
        // you passed an unknown parameter type.
        apply_param(std::forward<P0>(p0));

        apply_params(std::forward<Pn>(pn)...);
    }

    BOOST_HTTP_PROTO_DECL void apply_param(config_base const&) noexcept;

    // in detail/impl/brotli_codec.ipp
    BOOST_HTTP_PROTO_EXT_DECL void apply_param(brotli_decoder_t const&);

    // in detail/impl/zlib_codec.ipp
    BOOST_HTTP_PROTO_ZLIB_DECL void apply_param(deflate_decoder_t const&);
    BOOST_HTTP_PROTO_ZLIB_DECL void apply_param(gzip_decoder_t const&);

    detail::header const* safe_get_header() const;
    void parse_body(error_code&);
    void parse_chunk(error_code&);

    friend class request_parser;
    friend class response_parser;

    enum
    {
        br_codec = 0,
        deflate_codec = 1,
        gzip_codec = 2
    };

    enum class state
    {
        // order matters
        need_start,
        headers,        // header fields
        headers_done,   // delivered headers
        body,           // reading payload
        complete,       // done
    };

    config_base cfg_;
    detail::header h_;
    detail::workspace ws_;
    detail::header::config cfg_impl_;

    std::unique_ptr<
        detail::codec> dec_[3];
    detail::flat_buffer h_buf_;
    detail::circular_buffer b_buf_;
    detail::circular_buffer c_buf_;
    detail::codec* cod_;

    state st_;
    bool got_eof_;
    bool head_response_;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/parser.hpp>

#endif
