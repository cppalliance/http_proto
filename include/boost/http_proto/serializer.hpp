//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_SERIALIZER_HPP

#include <boost/http_proto/detail/array_of_const_buffers.hpp>
#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/source.hpp>

#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/rts/context_fwd.hpp>
#include <boost/system/result.hpp>

#include <type_traits>
#include <utility>

namespace boost {
namespace http_proto {

// Forward declaration
class message_view_base;
namespace detail {
class serializer_service;
class filter;
} // detail

/** A serializer for HTTP/1 messages

    This is used to serialize one or more complete
    HTTP/1 messages. Each message consists of a
    required header followed by an optional body.

    Objects of this type operate using an "input area" and an
    "output area". Callers provide data to the input area
    using one of the @ref start or @ref start_stream member
    functions. After input is provided, serialized data
    becomes available in the serializer's output area in the
    form of a constant buffer sequence.

    Callers alternate between filling the input area and
    consuming the output area until all the input has been
    provided and all the output data has been consumed, or
    an error occurs.

    After calling @ref start, the caller must ensure that the
    contents of the associated message are not changed or
    destroyed until @ref is_done returns true, @ref reset is
    called, or the serializer is destroyed, otherwise the
    behavior is undefined.
*/
class serializer
{
public:
    class stream;
    struct config;

    /** The type used to represent a sequence of
        constant buffers that refers to the output
        area.
    */
    using const_buffers_type =
        buffers::const_buffer_span;

    /** Constructor.

        Constructs a serializer that uses the @ref
        config parameters installed on the
        provided `ctx`.

        The serializer will attempt to allocate
        the required space on startup, with the
        amount depending on the @ref config
        parameters, and will not perform any
        further allocations, except for Brotli
        encoder instances, if enabled.

        Depending on which compression algorithms
        are enabled in the @ref config, the
        serializer will attempt to access the
        corresponding encoder services on the same
        `ctx`.

        @par Example
        @code
        serializer sr(ctx);
        @endcode

        @par Postconditions
        @code
        this->is_done() == true
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        Calls to allocate may throw.

        @param ctx Context from which the
        serializer will access registered
        services. The caller is responsible for
        ensuring that the provided ctx remains
        valid for the lifetime of the serializer.

        @see
            @ref install_serializer_service,
            @ref config.
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    serializer(
        const rts::context& ctx);

    /** Constructor.

        The states of `other` are transferred
        to the newly constructed object,
        which includes the allocated buffer.
        After construction, the moved-from object
        left in a valid but unspecified state.

        Buffer sequences previously obtained
        using @ref prepare or @ref stream::prepare
        remain valid.

        @par Postconditions
        @code
        other.is_done() == true
        @endcode

        @par Complexity
        Constant.

        @param other The serializer to move from.
    */
    BOOST_HTTP_PROTO_DECL
    serializer(
        serializer&& other) noexcept;

    /** Assignment.

        The states of `other` are transferred
        `this`, which includes the allocated buffer.
        The previous state of `this` are destroyed.
        After assignment, the moved-from object
        left in a valid but unspecified state.

        @par Postconditions
        @code
        other.is_done() == true
        @endcode

        @par Complexity
        Constant.

        @param other The serializer to assign from.
        @return A reference to this object.
    */
    BOOST_HTTP_PROTO_DECL
    serializer&
    operator=(
        serializer&& other) noexcept;

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    /** Reset the serializer for a new message.

        Aborts any ongoing serialization and
        prepares the serializer to start
        serialization of a new message.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset() noexcept;

    /** Prepare the serializer for a new message without a body.

        Initializes the serializer with the HTTP
        start-line and headers from `m`, and
        without a body.

        @par Preconditions
        @code
        this->is_done() == true
        @endcode

        @par Postconditions
        @code
        this->is_done() == false
        @endcode

        @par Exception Safety
        Strong guarantee.
        Exceptions thrown if there is insufficient
        internal buffer space to start the
        operation.

        @throw std::logic_error
        `this->is_done() == true`.

        @throw std::length_error if there is
        insufficient internal buffer space to
        start the operation.

        @param m The message to read the HTTP
        start-line and headers from.

        @see
            @ref message_view_base.
    */
    void
    start(
        message_view_base const& m)
    {
        start_empty(m);
    }

    /** Prepare the serializer for a new message with a ConstBufferSequence body.

        Initializes the serializer with the HTTP
        start-line and headers from `m`, and the
        provided `buffers` for reading the
        message body from.

        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.

        @par Preconditions
        @code
        this->is_done() == true
        @endcode

        @par Postconditions
        @code
        this->is_done() == false
        @endcode

        @par Constraints
        @code
        buffers::is_const_buffer_sequence<ConstBufferSequence>::value == true
        @endcode

        @par Exception Safety
        Strong guarantee.
        Exceptions thrown if there is insufficient
        internal buffer space to start the
        operation.

        @throw std::logic_error
        `this->is_done() == true`.

        @throw std::length_error if there is
        insufficient internal buffer space to
        start the operation.

        @param m The message to read the HTTP
        start-line and headers from.

        @param buffers One or more buffers
        containing the message body data. While
        the buffers object is copied, ownership of
        the underlying memory remains with the
        caller, who must ensure it stays valid
        until @ref is_done returns `true`.

        @see
            @ref message_view_base.
    */
    template<
        class ConstBufferSequence,
        class = typename std::enable_if<
            buffers::is_const_buffer_sequence<
                ConstBufferSequence>::value>::type
    >
    void
    start(
        message_view_base const& m,
        ConstBufferSequence&& buffers);

    /** Prepare the serializer for a new message with a Source body.

        Initializes the serializer with the
        HTTP start-line and headers from `m`,
        and constructs a `Source` object to read
        the message body.

        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.

        The serializer destroys Source object when:
        @li `this->is_done() == true`
        @li An unrecoverable serialization error occurs
        @li The serializer is destroyed

        @par Example
        @code
        file f;
        system::error_code ec;
        f.open("example.zip", file_mode::scan, ec);
        if(ec.failed())
            throw system::system_error(ec);
        serializer.start<file_source>(response, std::move(file));
        @endcode

        @par Preconditions
        @code
        this->is_done() == true
        @endcode

        @par Postconditions
        @code
        this->is_done() == false
        @endcode

        @par Constraints
        @code
        is_source<Source>::value == true
        @endcode

        @par Exception Safety
        Strong guarantee.
        Exceptions thrown if there is insufficient
        internal buffer space to start the
        operation.

        @throw std::length_error if there is
        insufficient internal buffer space to
        start the operation.

        @param m The message to read the HTTP
        start-line and headers from.

        @param args Arguments to be passed to the
        `Source` constructor.

        @return A reference to the costructed Source object.

        @see
            @ref source,
            @ref file_source,
            @ref message_view_base.
    */
    template<
        class Source,
        class... Args,
        class = typename std::enable_if<
            is_source<Source>::value>::type>
    Source&
    start(
        message_view_base const& m,
        Args&&... args);

    /** Prepare the serializer for a new message using a stream interface.

        Initializes the serializer with the HTTP
        start-line and headers from `m`, and returns
        a @ref stream object for reading the body
        from an external source.

        Once the serializer is destroyed, @ref reset
        is called, or @ref is_done returns true, the
        only valid operation on the stream is destruction.

        The stream allows inverted control flow: the
        caller supplies body data via the serializer’s
        internal buffer while reading from an external
        source.

        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.

        @par Example
        @code
        serializer::stream strm = serializer.start_stream(response);
        do
        {
            if(strm.is_open())
            {
                std::size_t n = source.read_some(strm.prepare());

                if(ec == error::eof)
                    strm.close();
                else
                    strm.commit(n);
            }

            write_some(client, serializer);

        } while(!serializer.is_done());
        @endcode

        @par Preconditions
        @code
        this->is_done() == true
        @endcode

        @par Postconditions
        @code
        this->is_done() == false
        @endcode

        @par Exception Safety
        Strong guarantee.
        Exceptions thrown if there is insufficient
        internal buffer space to start the
        operation.

        @throw std::length_error if there is
        insufficient internal buffer space to
        start the operation.

        @param m The message to read the HTTP
        start-line and headers from.

        @return A @ref stream object for reading body
        content into the serializer's buffer.

        @see
            @ref stream,
            @ref message_view_base.
     */
    BOOST_HTTP_PROTO_DECL
    stream
    start_stream(
        message_view_base const& m);

    /** Return the output area.

        This function serializes some or all of
        the message and returns the corresponding
        output buffers. Afterward, a call to @ref
        consume is required to report the number
        of bytes used, if any.

        If the message includes an
        `Expect: 100-continue` header and the
        header section of the message has been
        consumed, the returned result will contain
        @ref error::expect_100_continue to
        indicate that the header part of the
        message is complete. The next call to @ref
        prepare will produce output.

        When the serializer is used through the
        @ref stream interface, the result may
        contain @ref error::need_data to indicate
        that additional input is required to
        produce output.

        If a @ref source object is in use and a
        call to @ref source::read returns an
        error, the serializer enters a faulted
        state and propagates the error to the
        caller. This faulted state can only be
        cleared by calling @ref reset. This
        ensures the caller is explicitly aware
        that the previous message was truncated
        and that the stream must be terminated.

        @par Preconditions
        @code
        this->is_done() == false
        @endcode
        No unrecoverable error reported from previous calls.

        @par Exception Safety
        Strong guarantee.
        Calls to @ref source::read may throw if in use.

        @throw std::logic_error
        `this->is_done() == true`.

        @return A result containing @ref
        const_buffers_type that represents the
        output area or an error if any occurred.

        @see
            @ref consume,
            @ref is_done,
            @ref const_buffers_type.
    */
    BOOST_HTTP_PROTO_DECL
    auto
    prepare() ->
        system::result<
            const_buffers_type>;

    /** Consume bytes from the output area.

        This function should be called after one
        or more bytes contained in the buffers
        provided in the prior call to @ref prepare
        have been used.

        After a call to @ref consume, callers
        should check the return value of @ref
        is_done to determine if the entire message
        has been serialized.

        @par Preconditions
        @code
        this->is_done() == false
        @endcode

        @par Exception Safety
        Strong guarantee.

        @throw std::logic_error
        `this->is_done() == true`.

        @param n The number of bytes to consume.
        If `n` is greater than the size of the
        buffer returned from @ref prepared the
        entire output sequence is consumed and no
        error is issued.

        @see
            @ref prepare,
            @ref is_done,
            @ref const_buffers_type.
    */
    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n);

    /** Return true if serialization is complete.
    */
    bool
    is_done() const noexcept
    {
        return state_ == state::start;
    }

private:
    class cbs_gen;
    template<class>
    class cbs_gen_impl;

    detail::array_of_const_buffers
    make_array(std::size_t n);

    template<
        class Source,
        class... Args,
        typename std::enable_if<
            std::is_constructible<
                Source,
                Args...>::value>::type* = nullptr>
    Source&
    construct_source(Args&&... args)
    {
        return ws_.emplace<Source>(
            std::forward<Args>(args)...);
    }

    template<
        class Source,
        class... Args,
        typename std::enable_if<
            std::is_constructible<
                Source,
                detail::workspace&,
                Args...>::value>::type* = nullptr>
    Source&
    construct_source(Args&&... args)
    {
        return ws_.emplace<Source>(
            ws_, std::forward<Args>(args)...);
    }

    BOOST_HTTP_PROTO_DECL
    void
    start_init(
        message_view_base const&);

    BOOST_HTTP_PROTO_DECL
    void
    start_empty(
        message_view_base const&);

    BOOST_HTTP_PROTO_DECL
    void
    start_buffers(
        message_view_base const&);

    BOOST_HTTP_PROTO_DECL
    void
    start_source(
        message_view_base const&);

    bool
    is_header_done() const noexcept;

    void
    out_init();

    buffers::mutable_buffer_pair
    out_prepare() noexcept;

    void
    out_commit(std::size_t) noexcept;

    std::size_t
    out_capacity() const noexcept;

    void
    out_finish() noexcept;

    enum class state
    {
        reset,
        start,
        header,
        body
    };

    enum class style
    {
        empty,
        buffers,
        source,
        stream
    };

    const rts::context* ctx_;
    detail::serializer_service* svc_;
    detail::workspace ws_;

    detail::filter* filter_ = nullptr;
    cbs_gen* cbs_gen_ = nullptr;
    source* source_ = nullptr;

    buffers::circular_buffer out_;
    buffers::circular_buffer in_;
    detail::array_of_const_buffers prepped_;
    buffers::const_buffer tmp_;

    state state_ = state::start;
    style style_ = style::empty;
    uint8_t chunk_header_len_ = 0;
    bool more_input_ = false;
    bool is_chunked_ = false;
    bool needs_exp100_continue_ = false;
    bool filter_done_ = false;
};

/** Serializer configuration settings.

    @see
        @ref install_serializer_service,
        @ref serializer.
*/
struct serializer::config
{
    /** Enable Brotli Content-Encoding.

        Requires `boost::rts::brotli::encode_service` to be
        installed, otherwise an exception is thrown.
    */
    bool apply_brotli_encoder = false;

    /** Enable Deflate Content-Encoding.

        Requires `boost::zlib::deflate_service` to be
        installed, otherwise an exception is thrown.
    */
    bool apply_deflate_encoder = false;

    /** Enable Gzip Content-Encoding.

        Requires `boost::zlib::deflate_service` to be
        installed, otherwise an exception is thrown.
    */
    bool apply_gzip_encoder = false;

    /** Brotli compression quality (0–11).

        Higher values yield better but slower compression.
    */
    std::uint32_t brotli_comp_quality = 5;

    /** Brotli compression window size (10–24).

        Larger windows improve compression but increase
        memory usage.
    */
    std::uint32_t brotli_comp_window = 18;

    /** Zlib compression level (0–9).

        0 = no compression, 1 = fastest, 9 = best
        compression.
    */
    int zlib_comp_level = 6;

    /** Zlib window bits (9–15).

        Controls the history buffer size. Larger values
        improve compression but use more memory.
    */
    int zlib_window_bits = 15;

    /** Zlib memory level (1–9).

        Higher values use more memory, but offer faster
        and more efficient compression.
    */
    int zlib_mem_level = 8;

    /** Minimum buffer size for payloads (must be > 0). */
    std::size_t payload_buffer = 8192;

    /** Reserved space for type-erasure storage.

        Used for:
        @li User-defined @ref source objects.
        @li User-defined ConstBufferSequence instances.
    */
    std::size_t max_type_erase = 1024;
};

/** Install the serializer service.

    @par Example
    @code
    // default configuration settings
    install_serializer_service(ctx, {});

    serializer sr(ctx);
    @endcode

    @par Exception Safety
    Strong guarantee.

    @throw std::invalid_argument If the service is
    already installed on the context.

    @param ctx Reference to the context on which
    the service should be installed.

    @param cfg Configuration settings for the
    serializer.

    @see
        @ref serializer::config,
        @ref serializer.
*/
BOOST_HTTP_PROTO_DECL
void
install_serializer_service(
    rts::context& ctx,
    serializer::config const& cfg);

//------------------------------------------------

/** Used for streaming body data during serialization.

    Provides an interface for supplying serialized
    body content from an external source. This
    object is returned by @ref
    serializer::start_stream and enables
    incremental writing of the message body into
    the serializer's internal buffer.

    The stream supports an inverted control flow
    model, where the caller pushes body data as
    needed.

    Valid operations depend on the state of the
    serializer. Once the serializer is destroyed,
    reset, or completes, the stream becomes
    invalid and must only be destroyed.

    @see
        @ref serializer::start_stream
*/
class serializer::stream
{
public:
    /** The type used to represent a sequence
        of mutable buffers.
    */
    using mutable_buffers_type =
        buffers::mutable_buffer_pair;

    /** Constructor.

        A default-constructed stream is
        considered closed.

        @par Postconditions
        @code
        this->is_open() == false
        @endcode
    */
    stream() noexcept = default;

    /** Constructor.

        After construction, the moved-from
        object is as if default-constructed.

        @par Postconditions
        @code
        other->is_open() == false
        @endcode

        @param other The object to move from.
    */
    stream(stream&& other) noexcept
        : sr_(other.sr_)
    {
        other.sr_ = nullptr;
    }

    /** Move assignment.

        After assignment, the moved-from
        object is as if default-constructed.

        @par Postconditions
        @code
        other->is_open() == false
        @endcode

        @param other The object to assign from.
        @return A reference to this object.
    */
    stream&
    operator=(stream&& other) noexcept
    {
        std::swap(sr_, other.sr_);
        return *this;
    }

    /** Return true if the stream is open.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    is_open() const noexcept;

    /** Return the available capacity.

        @par Preconditions
        @code
        this->is_open() == true
        @endcode

        @par Exception Safety
        Strong guarantee.

        @throw std::logic_error
        `this->is_open() == false`.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    capacity() const;

    /** Prepare a buffer for writing.

        Retuns a mutable buffer sequence representing
        the writable bytes. Use @ref commit to make the
        written data available to the serializer.

        All buffer sequences previously obtained
        using @ref prepare are invalidated.

        @par Preconditions
        @code
        this->is_open() == true && n <= this->capacity()
        @endcode

        @par Exception Safety
        Strong guarantee.

        @return An instance of @ref mutable_buffers_type
        the underlying memory is owned by the serializer.

        @throw std::logic_error
        `this->is_open() == false`

        @see
            @ref commit,
            @ref capacity.
    */
    BOOST_HTTP_PROTO_DECL
    mutable_buffers_type
    prepare();

    /** Commit data to the serializer.

        Makes `n` bytes avialable to the serializer.

        All buffer sequences previously obtained
        using @ref prepare are invalidated.

        @par Preconditions
        @code
        this->is_open() == true && n <= this->capacity()
        @endcode

        @par Exception Safety
        Strong guarantee.
        Exceptions thrown on invalid input.

        @param n The number of bytes to append.

        @throw std::invalid_argument
        `n > this->capacity()`

        @throw std::logic_error
        `this->is_open() == false`

        @see
            @ref prepare,
            @ref capacity.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(std::size_t n);

    /** Close the stream if open.

        Closes the stream and
        notifies the serializer that the
        message body has ended.

        If the stream is already closed this
        call has no effect.

        @par Postconditions
        @code
        this->is_open() == false
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    void
    close() noexcept;

    /** Destructor.

        Closes the stream if open.
    */
    BOOST_HTTP_PROTO_DECL
    ~stream();

private:
    friend class serializer;

    explicit
    stream(
        serializer& sr) noexcept
        : sr_(&sr)
    {
    }

    serializer* sr_ = nullptr;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
