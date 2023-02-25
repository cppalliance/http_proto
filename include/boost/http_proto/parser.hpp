//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_PARSER_HPP
#define BOOST_HTTP_PROTO_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/header_limits.hpp>
#include <boost/http_proto/sink.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/flat_buffer.hpp>
#include <boost/buffers/mutable_buffer_pair.hpp>
#include <boost/buffers/mutable_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/url/grammar/error.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class parser_service;
class filter;
class request_parser;
class response_parser;
class context;

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
    BOOST_HTTP_PROTO_DECL
    parser(context& ctx, detail::kind);

public:
    /** Parser configuration settings

        @see
            @li <a href="https://stackoverflow.com/questions/686217/maximum-on-http-header-values"
                >Maximum on HTTP header values (Stackoverflow)</a>
    */
    struct config_base
    {
        header_limits headers;

        /** Largest allowed size for a content body.

            The size of the body is measured
            after removing any transfer encodings,
            including a chunked encoding.
        */
        std::uint64_t body_limit = 64 * 1024;

        /** True if parser can decode deflate transfer and content encodings.

            The deflate decoder must already be
            installed thusly, or else an exception
            is thrown.

            @par Install Deflate Decoder
            @code
            deflate_decoder_service::config cfg;
            cfg.install( ctx );
            @endcode
        */
        bool apply_deflate_decoder = false;

        /** Minimum space for payload buffering.

            This value controls the following
            settings:

            @li The smallest allocated size of
                the buffers used for reading
                and decoding the payload.

            @li The lowest guaranteed size of
                an in-place body.

            @li The largest size used to reserve
                space in dynamic buffer bodies
                when the payload size is not
                known ahead of time.

            This cannot be zero, and this cannot
            be greater than @ref body_limit.
        */
        std::size_t min_buffer = 4096;

        /** Largest permissible output size in prepare.

            This cannot be zero.
        */
        std::size_t max_prepare = std::size_t(-1);

        /** Space to reserve for type-erasure.
        */
        std::size_t max_type_erase = 1024;
    };

    using mutable_buffers_type =
        buffers::mutable_buffer_span;

    struct stream;

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Destructor.
    */
    BOOST_HTTP_PROTO_DECL
    ~parser();

    /** Constructor.
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
        return st_ > state::header;
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
    mutable_buffers_type
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

    /** Attach a body
    */
    // VFALCO Should this function have
    //        error_code& ec and call parse?
    template<class DynamicBuffer>
#ifndef BOOST_HTTP_PROTO_DOCS
    typename std::enable_if<
        buffers::is_dynamic_buffer<
            DynamicBuffer>::value,
        typename std::decay<
            DynamicBuffer>::type
                >::type
#else
    typename std::decay<
        DynamicBuffer>::type
#endif
    set_body(DynamicBuffer&& b);

    /** Attach a body
    */
    // VFALCO Should this function have
    //        error_code& ec and call parse?
    template<class Sink>
#ifndef BOOST_HTTP_PROTO_DOCS
    typename std::enable_if<
        is_sink<Sink>::value,
        typename std::decay<Sink>::type
            >::type
#else
    typename std::decay<Sink>::type
#endif
    set_body(Sink&& sink);

    /** Return a stream for receiving body data.
    */
    BOOST_HTTP_PROTO_DECL
    stream
    get_stream();

    BOOST_HTTP_PROTO_DECL
    string_view
    in_place_body() const;

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
    friend class request_parser;
    friend class response_parser;

    detail::header const*
        safe_get_header() const;
    void on_headers(error_code&);
    void on_set_body();

    template<class T>
    struct any_dynamic_impl;
    struct any_dynamic;
    static constexpr unsigned dynamic_N_ = 8;

    enum class state
    {
        // order matters
        reset,
        start,
        header,
        body,
        complete,
    };

    enum class body
    {
        in_place,
        dynamic,
        sink,
        stream
    };

    context& ctx_;
    parser_service& svc_;
    detail::workspace ws_;
    detail::header h_;

    buffers::flat_buffer fb_;
    buffers::circular_buffer cb0_;
    buffers::circular_buffer cb1_;
    buffers::circular_buffer* body_buf_;
    buffers::mutable_buffer_pair mbp_;
    any_dynamic* dyn_;
    filter* filt_;
    sink* sink_;

    state st_;
    body body_;
    bool got_eof_;
    bool head_response_;
};

//------------------------------------------------

struct parser::stream
{
    /** Constructor.
    */
    stream() = default;

    /** Constructor.
    */
    stream(stream const&) = default;

    /** Constructor.
    */
    stream& operator=
        (stream const&) = default;

    using buffers_type =
        buffers::const_buffer_pair;

    BOOST_HTTP_PROTO_DECL
    buffers_type
    data() const noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n);

private:
    friend class parser;

    explicit
    stream(
        parser& pr) noexcept
        : pr_(&pr)
    {
    }

    parser* pr_ = nullptr;
};

//------------------------------------------------

/** Install the parser service.
*/
BOOST_HTTP_PROTO_DECL
void
install_parser_service(
    context& ctx,
    parser::config_base const& cfg);

} // http_proto
} // boost

#include <boost/http_proto/impl/parser.hpp>

#endif
