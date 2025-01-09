//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_PARSER_HPP
#define BOOST_HTTP_PROTO_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/type_traits.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/header_limits.hpp>
#include <boost/http_proto/sink.hpp>

#include <boost/buffers/any_dynamic_buffer.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/flat_buffer.hpp>
#include <boost/buffers/mutable_buffer_pair.hpp>
#include <boost/buffers/mutable_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/url/grammar/error.hpp>

#include <cstddef>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class parser_service;
class request_parser;
class response_parser;
class context;
namespace detail {
class filter;
} // detail
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

            The zlib service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_deflate_decoder = false;

        /** True if parser can decode gzip transfer and content encodings.

            The zlib service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_gzip_decoder = false;

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

    using const_buffers_type =
        buffers::const_buffer_span;

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

    /** Constructor (deleted)
    */
    parser(parser&&) = delete;

    /** Assignment (deleted)
    */
    parser& operator=(parser&&) = delete;

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
        return st_ >= state::complete_in_place;
    }

    /** Returns `true` if the end of the stream was reached.

        The end of the stream is encountered
        when one of the following conditions
        occurs:

        @li @ref commit_eof was called and there
            is no more data left to parse, or

        @li An unrecoverable error occurred
            during parsing.

        When the end of stream is reached, the
            function @ref reset must be called
            to start parsing a new stream.
    */
    bool
    is_end_of_stream() const noexcept
    {
        return
            st_ == state::reset ||
            (st_ >= state::complete_in_place && got_eof_);
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

    /** Prepare for the next message on the stream.
    */
    void
    start()
    {
        start_impl(false);
    }

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

        @par Postconditions
        All buffer sequences previously obtained
        by calling @ref prepare are invalidated.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit_eof();

    /** Parse pending input data
    */
    // VFALCO return result<void>?
    BOOST_HTTP_PROTO_DECL
    void
    parse(
        system::error_code& ec);

    /** Attach a body.

        This function attaches the specified elastic
        buffer as the storage for the message body.
        The parser acquires ownership of the object
        `eb` and destroys it when:

        @li @ref is_complete returns `true`, or
        @li @ref reset is called, or
        @li an unrecoverable parsing error occurs, or
        @li the parser is destroyed.
    */
    // VFALCO Should this function have
    //        error_code& ec and call parse?
    template<class ElasticBuffer>
#ifndef BOOST_HTTP_PROTO_DOCS
    typename std::enable_if<
        ! detail::is_reference_wrapper<
            ElasticBuffer>::value &&
        ! is_sink<ElasticBuffer>::value>::type
#else
    void
#endif
    set_body(ElasticBuffer&& eb);

    /** Attach a body.

        This function attaches the specified elastic
        buffer reference as the storage for the message body.
        Ownership is not transferred; the caller must
        ensure that the lifetime of the object
        reference by `eb` extends until:

        @li @ref is_complete returns `true`, or
        @li @ref reset is called, or
        @li an unrecoverable parsing error occurs, or
        @li the parser is destroyed.
    */
    template<class ElasticBuffer>
    void set_body(
        std::reference_wrapper<ElasticBuffer> eb);

    /** Attach a body
    */
    template<
        class Sink,
        class... Args
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename std::enable_if<
            is_sink<Sink>::value>::type
#endif
    >
    Sink&
    set_body(Args&&... args);

    /** Sets the maximum allowed size of the body for the current message.

        This overrides the default value specified by
        @ref config_base::body_limit.
        The limit automatically resets to the default
        for the next message.

        @param n The new body size limit in bytes.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_body_limit(std::uint64_t n);

    /** Return the available body data.

        The returned buffer span will be invalidated if any member
        function of the parser is subsequently called.
    */
    BOOST_HTTP_PROTO_DECL
    const_buffers_type
    pull_body();

    /** Consumes bytes from the available body data.
    */
    BOOST_HTTP_PROTO_DECL
    void
    consume_body(std::size_t n);

    /** Return the complete body as a contiguous character buffer.
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    body() const noexcept;

    //--------------------------------------------

    /** Return any leftover data

        This is used to forward unconsumed data
        that could lie past the last message.
        For example on a CONNECT request there
        could be additional protocol-dependent
        data that we want to retrieve.
    */
    // VFALCO rename to get_leftovers()?
    BOOST_HTTP_PROTO_DECL
    core::string_view
    release_buffered_data() noexcept;

private:
    friend class request_parser;
    friend class response_parser;

    detail::header const*
    safe_get_header() const;

    bool
    is_plain() const noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    on_set_body() noexcept;

    std::size_t
    apply_filter(
        system::error_code&,
        std::size_t,
        bool);

    std::uint64_t
    body_limit_remain() const noexcept;

    static constexpr unsigned buffers_N = 8;

    enum class state
    {
        reset,
        start,
        header,
        header_done,
        body,
        set_body,
        complete_in_place,
        complete
    };

    enum class how
    {
        in_place,
        sink,
        elastic,
    };

    context& ctx_;
    parser_service& svc_;

    detail::workspace ws_;
    detail::header h_;
    std::uint64_t body_limit_= 0;
    std::uint64_t body_total_ = 0;
    std::uint64_t payload_remain_ = 0;
    std::uint64_t chunk_remain_ = 0;
    std::size_t body_avail_ = 0;
    std::size_t nprepare_ = 0;

    // used to store initial headers + any potential overread
    buffers::flat_buffer fb_;

    // used for raw input once headers are read
    buffers::circular_buffer cb0_;

    // used for transformed output, if applicable
    // can be empty/null
    buffers::circular_buffer cb1_;

    // used to provide stable storage when returning
    // `mutable_buffers_type` from relevant functions
    buffers::mutable_buffer_pair mbp_;

    // used to provide stable storage when returning
    // `const_buffers_type` from relevant functions
    buffers::const_buffer_pair cbp_;

    buffers::any_dynamic_buffer* eb_ = nullptr;
    detail::filter* filter_ = nullptr;
    sink* sink_ = nullptr;

    state st_ = state::start;
    how how_ = how::in_place;
    bool got_eof_ = false;
    bool head_response_ = false;
    bool needs_chunk_close_ = false;
    bool trailer_headers_ = false;
    bool chunked_body_ended = false;
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
