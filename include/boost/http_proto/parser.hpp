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
#include <boost/rts/context_fwd.hpp>
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

    This parser uses a single block of memory allocated
    during construction and guarantees that it will
    never exceed the specified size. This space will be
    reused for parsing multiple HTTP messages (one
    message at a time).

    The allocated space will be utilized for the
    following purposes:

    @li Provide a mutable buffer sequence for reading
        raw input (for example, from a socket).
    @li Storing HTTP headers and provide a non-owning,
        read-only view that allows efficient access and
        iteration through header names and values.
    @li Offering O(1) access to important HTTP headers,
        including the request method, target, and
        response status code.
    @li Storing all or part of an HTTP message and
        provide the necessary interfaces for retrieving
        it.
    @li Taking ownership of user-provided elastic
        buffer and Sink objects.
    @li Storing the necessary state for inflate
        algorithms.

    The parser is strict. Any malformed inputs
    according to the documented HTTP ABNFs is treated
    as an unrecoverable error.
*/
class parser
{
public:
    /** Parser configuration settings.
    */
    struct config_base
    {
        /** Configurable limits for HTTP headers.
        */
        header_limits headers;

        /** Largest allowed size for a content body.

            The size of the body is measured
            after removing any transfer encodings,
            including a chunked encoding.
        */
        std::uint64_t body_limit = 64 * 1024;


        /** True if parser can decode br Content-Encoding.

            The @ref rts::brotli::decode_service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_brotli_decoder = false;

        /** True if parser can decode deflate Content-Encoding.

            The @ref rts::zlib::inflate_service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_deflate_decoder = false;

        /** True if parser can decode gzip Content-Encoding.

            The @ref zrts::lib::inflate_service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_gzip_decoder = false;

        /** Specifies the zlib windows bits 9..15.

            The windows bits must be greater than or equal to
            the windows bits value used for compression.

            If a compressed message has a larger window size,
            parsing ends with @ref zlib::error::data_err instead
            of allocating a bigger window.
        */
        int zlib_window_bits = 15;

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

            This cannot be zero.
        */
        std::size_t min_buffer = 4096;

        /** Largest permissible output size in prepare.

            This cannot be zero.
        */
        std::size_t max_prepare = std::size_t(-1);

        /** Space to reserve for type-erasure.

            This space is used for the following
            purposes:

            @li Storing an instance of the user-provided
                @ref sink objects.

            @li Storing an instance of the user-provided
                ElasticBuffer.
        */
        std::size_t max_type_erase = 1024;
    };

    /** The type of buffer returned from @ref prepare.
    */
    using mutable_buffers_type =
        buffers::mutable_buffer_span;

    /** The type of buffer returned from @ref pull_body.
    */
    using const_buffers_type =
        buffers::const_buffer_span;

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor (deleted)
    */
    parser(parser&&) = delete;

    /** Assignment (deleted)
    */
    parser& operator=(parser&&) = delete;

    /** Destructor.
    */
    BOOST_HTTP_PROTO_DECL
    ~parser();

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Returns `true` if a complete header has been
        parsed.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    got_header() const noexcept;

    /** Returns `true` if a complete message has been
        parsed.

        Calling @ref start prepares the parser
        to process the next message in the stream.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    is_complete() const noexcept;

#if 0
    /** Return true if any input was committed.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    got_some() const noexcept;

    /** Returns `true` if the end of the stream was reached.

        The end of the stream is encountered
        when @ref commit_eof was called and there
        is no more data left to parse.

        When the end of stream is reached, the
        function @ref reset must be called
        to start parsing a new stream.
    */
    bool
    is_end_of_stream() const noexcept
    {
        return
            got_eof_ &&
            (st_ == state::reset || st_ >= state::complete_in_place);
    }
#endif

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Prepare for a new stream.

        This function must be called before parsing  
        the first message in a new stream.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset() noexcept;

    /** Prepare for a new message on the stream.

        This function must be called before parsing
        a new message in a stream.

        @par Preconditions
        This function may only be called if it is the
        first message being read from the stream or if
        the previous message has been fully parsed.
    */
    BOOST_HTTP_PROTO_DECL
    void
    start();

    /** Prepares the input buffer.

        The returned buffer sequence will either
        reference the internal buffer or, if available,
        the attached elastic buffer.

        @par Preconditions
        This function may only be called after a call
        to @ref parse completes with an error code
        equal to @ref condition::need_more_input.

        @return A non-empty mutable buffer.

        @see
            @ref commit,
            @ref commit_eof.
    */
    BOOST_HTTP_PROTO_DECL
    mutable_buffers_type
    prepare();

    /** Commit bytes to the input buffer

        After committing, a call to @ref parse
        is required to process the input.

        @par Preconditions
        @li `n` must be less than or equal to
            the size of the buffer returned
            by @ref prepare.
        @li No previous call to @ref commit.
        @li No previous call to @ref commit_eof.

        @par Postconditions
        All buffer sequences previously obtained  
        from @ref prepare are invalidated.

        @param n The number of bytes written to
        the input buffer.

        @see
            @ref parse.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(
        std::size_t n);

    /** Indicate there will be no more input.

        @par Postconditions
        All buffer sequences previously obtained
        from @ref prepare are invalidated.

        @see
            @ref parse.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit_eof();

    /** Parse pending input data

        This function attempts to parse the pending
        input data.

        This function returns immediately after the
        header is fully parsed. This is because certain
        operations, like @ref set_body_limit, must be
        performed before the body parsing starts. It is
        also more efficient to attach the body at this
        stage to avoid extra copy operations. The body
        parsing will begin in a subsequent call.

        If an error occurs during body parsing, the
        parsed header will remain valid and accessible.

        If @ref set_body was called previously, this
        function first tries to transfer available
        body data to the Sink or elastic buffer.

        When `ec == condition::need_more`, more input
        needs to be read into the internal buffer
        before continuing parsing.

        When `ec == error::end_of_stream`, all
        messages have been parsed, and the stream has
        closed cleanly. The parser can be reused for  
        a new stream after a call to @ref reset.

        @param ec Set to the error, if any occurred.

        @see
            @ref start,
            @ref prepare,
            @ref commit,
            @ref commit_eof.
    */
    BOOST_HTTP_PROTO_DECL
    void
    parse(
        system::error_code& ec);

    /** Attach an elastic buffer body.

        This function attaches the specified elastic
        buffer as the storage for the message body.

        A call to @ref parse is required after this
        function for the changes to take effect. This
        should automatically happen during the next
        IO layer call when reading the body.

        The parser takes ownership of the `eb` object and
        will destroy it when one of the following occurs:
        @li The message body is completely received, or
        @li An unrecoverable parsing error occurs, or
        @li The parser is destroyed.

        @par Preconditions
        @li Header has been completely parsed.
        @li No body is already attached.

        @see
            @ref parse.
    */
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

    /** Attach a reference to an elastic buffer body.

        This function attaches the specified elastic
        buffer reference as the storage for
        the message body.

        A call to @ref parse is required after this
        function for the changes to take effect. This
        should automatically happen during the next
        IO layer call when reading the body.

        Ownership is not transferred; the caller must
        ensure that the lifetime of the object
        reference by `eb` extends until:
        @li The message body is completely received, or
        @li An unrecoverable parsing error occurs, or
        @li The parser is destroyed.

        @par Preconditions
        @li Header has been completely parsed.
        @li No body is already attached.

        @see
            @ref parse.
    */
    template<class ElasticBuffer>
    void set_body(
        std::reference_wrapper<ElasticBuffer> eb);

    /** Attach a Sink body.

        This function constructs a Sink for transferring
        the message body to it.

        A call to @ref parse is required after this
        function for the changes to take effect. This
        should automatically happen during the next
        IO layer call when reading the body.

        The parser destroys Sink object when:
        @li The message body is completely received, or
        @li An unrecoverable parsing error occurs, or
        @li The parser is destroyed.

        @par Preconditions
        @li Header has been completely parsed.
        @li No body is already attached.

        @return A reference to the costructed Sink object.

        @see
            @ref parse.
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

    /** Sets the maximum allowed body size for
        the current message.

        This overrides the default value specified by
        @ref config_base::body_limit, but only for
        the current message. The limit automatically
        resets to the default for the next message.

        @par Preconditions
        This function can be called after
        @ref start and before parsing the body.

        @param n The new body size limit in bytes.  
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_body_limit(std::uint64_t n);

    /** Return the available body data.

        The returned buffer span may become invalid if
        any modifying member function is called.

        @par Preconditions
        This function can be called only after the
        header is fully parsed and no body is attached.

        @return An instance of @ref const_buffers_type
        containing the parsed body data.  
    */
    BOOST_HTTP_PROTO_DECL
    const_buffers_type
    pull_body();

    /** Consumes bytes from the available body data.

        @par Preconditions
        `n` must be less than or equal to the size of
        the buffer returned by @ref pull_body.

        @param n The number of bytes to consume from
        the available body data.
    */
    BOOST_HTTP_PROTO_DECL
    void
    consume_body(std::size_t n);

    /** Return the complete body as a contiguous buffer.

        @par Preconditions
        @li The message has been fully parsed.
        @li No body is currently attached.
        @li @ref consume_body has not been called before.

        @return A contiguous character buffer containing
        the complete body data.
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    body() const noexcept;

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

    BOOST_HTTP_PROTO_DECL
    parser(rts::context&, detail::kind);

    BOOST_HTTP_PROTO_DECL
    void
    start_impl(bool);

    BOOST_HTTP_PROTO_DECL
    void
    on_set_body() noexcept;

    std::size_t
    apply_filter(
        system::error_code&,
        std::size_t,
        bool);

    detail::header const*
    safe_get_header() const;

    bool
    is_plain() const noexcept;

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

    rts::context& ctx_;
    parser_service& svc_;

    detail::workspace ws_;
    detail::header h_;
    std::uint64_t body_limit_;
    std::uint64_t body_total_;
    std::uint64_t payload_remain_;
    std::uint64_t chunk_remain_;
    std::size_t body_avail_;
    std::size_t nprepare_;

    buffers::flat_buffer fb_;
    buffers::circular_buffer cb0_;
    buffers::circular_buffer cb1_;

    buffers::mutable_buffer_pair mbp_;
    buffers::const_buffer_pair cbp_;

    detail::filter* filter_;
    buffers::any_dynamic_buffer* eb_;
    sink* sink_;

    state st_;
    how how_;
    bool got_header_;
    bool got_eof_;
    bool head_response_;
    bool needs_chunk_close_;
    bool trailer_headers_;
    bool chunked_body_ended;
};

//------------------------------------------------

/** Install the parser service.
*/
BOOST_HTTP_PROTO_DECL
void
install_parser_service(
    rts::context& ctx,
    parser::config_base const& cfg);

} // http_proto
} // boost

#include <boost/http_proto/impl/parser.hpp>

#endif
