//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_SERIALIZER_HPP

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/detail/array_of_buffers.hpp>
#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/source.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/range.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/system/result.hpp>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class request;
class response;
class request_view;
class response_view;
class message_view_base;
namespace detail {
class filter;
} // detail
#endif

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
class BOOST_SYMBOL_VISIBLE
    serializer
{
public:
    using const_buffers_type = buffers::const_buffer_span;

    struct stream;

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    serializer(
        serializer&&) noexcept;

    /** Constructor

        @param ctx The serializer will access services
                   registered with this context.
    */
    BOOST_HTTP_PROTO_DECL
    serializer(
        context& ctx);

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    serializer(
        context& ctx,
        std::size_t buffer_size);

    //--------------------------------------------

    /** Prepare the serializer for a new stream
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset() noexcept;

    /** Prepare the serializer for a new message

        The message will not contain a body.
        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.
    */
    void
    start(
        message_view_base const& m)
    {
        start_empty(m);
    }

    /** Prepare the serializer for a new message

        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.

        @par Constraints
        @code
        is_const_buffers< ConstBuffers >::value == true
        @endcode
    */
    template<
        class ConstBufferSequence
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename
            std::enable_if<
                buffers::is_const_buffer_sequence<
                    ConstBufferSequence>::value
                        >::type
#endif
    >
    void
    start(
        message_view_base const& m,
        ConstBufferSequence&& body);

    /** Prepare the serializer for a new message

        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.
    */
    template<
        class Source,
        class... Args
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename std::enable_if<
            is_source<Source>::value>::type
#endif
    >
    Source&
    start(
        message_view_base const& m,
        Args&&... args);

    //--------------------------------------------

    /** Return a new stream for this serializer.

        After the serializer is destroyed, @ref reset is called,
        or @ref is_done returns true, the only valid operation
        on the stream is destruction.

        A stream may be used to invert the flow of control
        when the caller is supplying body data as a series
        of buffers.
     */
    BOOST_HTTP_PROTO_DECL
    stream
    start_stream(
        message_view_base const& m);

    //--------------------------------------------

    /** Return true if serialization is complete.
    */
    bool
    is_done() const noexcept
    {
        return is_done_;
    }

    /** Return the output area.

        This function will serialize some or
        all of the content and return the
        corresponding output buffers.

        @par Preconditions
        @code
        this->is_done() == false
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    auto
    prepare() ->
        system::result<
            const_buffers_type>;

    /** Consume bytes from the output area.
    */
    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n);

    /** Applies deflate compression to the current message

        After @ref reset is called, compression is not
        applied to the next message.

        Must be called before any calls to @ref start.
    */
    BOOST_HTTP_PROTO_DECL
    void
    use_deflate_encoding();

    /** Applies gzip compression to the current message

        After @ref reset is called, compression is not
        applied to the next message.

        Must be called before any calls to @ref start.
    */
    BOOST_HTTP_PROTO_DECL
    void
    use_gzip_encoding();

private:
    static void copy(
        buffers::const_buffer*,
        buffers::const_buffer const*,
        std::size_t n) noexcept;
    auto
    make_array(std::size_t n) ->
        detail::array_of_const_buffers;

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

    BOOST_HTTP_PROTO_DECL void start_init(message_view_base const&);
    BOOST_HTTP_PROTO_DECL void start_empty(message_view_base const&);
    BOOST_HTTP_PROTO_DECL void start_buffers(message_view_base const&);
    BOOST_HTTP_PROTO_DECL void start_source(message_view_base const&, source*);

    enum class style
    {
        empty,
        buffers,
        source,
        stream
    };

    // chunked-body   = *chunk
    //                  last-chunk
    //                  trailer-section
    //                  CRLF

    static
    constexpr
    std::size_t
    crlf_len_ = 2;

    // chunk          = chunk-size [ chunk-ext ] CRLF
    //                  chunk-data CRLF
    static
    constexpr
    std::size_t
    chunk_header_len_ =
        16 + // 16 hex digits => 64 bit number
        crlf_len_;

    // last-chunk     = 1*("0") [ chunk-ext ] CRLF
    static
    constexpr
    std::size_t
    last_chunk_len_ =
        1 + // "0"
        crlf_len_ +
        crlf_len_; // chunked-body termination requires an extra CRLF

    static
    constexpr
    std::size_t
    chunked_overhead_ =
        chunk_header_len_ +
        crlf_len_ + // closing chunk data
        last_chunk_len_;

    detail::workspace ws_;
    detail::array_of_const_buffers buf_;
    detail::filter* filter_ = nullptr;
    source* src_;
    context& ctx_;
    buffers::circular_buffer tmp0_;
    buffers::circular_buffer tmp1_;
    detail::array_of_const_buffers prepped_;

    buffers::mutable_buffer chunk_header_;
    buffers::mutable_buffer chunk_close_;
    buffers::mutable_buffer last_chunk_;

    buffers::circular_buffer* in_ = nullptr;
    buffers::circular_buffer* out_ = nullptr;

    buffers::const_buffer* hp_;  // header

    style st_;
    bool more_;
    bool is_done_;
    bool is_header_done_;
    bool is_chunked_;
    bool is_expect_continue_;
    bool is_compressed_ = false;
    bool filter_done_ = false;
};

//------------------------------------------------

/** The type used for caller-provided body data during
    serialization.

    @code{.cpp}
    http_proto::serializer sr(128);

    http_proto::request req;
    auto stream = sr.start_stream(req);

    std::string_view msg = "Hello, world!";
    auto n = buffers::copy(
        stream.prepare(),
        buffers::make_buffer(
            msg.data(), msg.size()));

    stream.commit(n);

    auto cbs = sr.prepare().value();
    (void)cbs;
    @endcode
*/
struct serializer::stream
{
    /** Constructor.

        The only valid operations on default constructed
        streams are assignment and destruction.
    */
    stream() = default;

    /** Constructor.

        The constructed stream will share the same
        serializer as `other`.
    */
    stream(stream const& other) = default;

    /** Assignment.

        The current stream will share the same serializer
        as `other`.
    */
    stream& operator= (
        stream const& other) = default;

    /** A MutableBufferSequence consisting of a buffer pair.
     */
    using buffers_type =
        buffers::mutable_buffer_pair;

    /** Returns the remaining available capacity.

        The returned value represents the available free
        space in the backing fixed-sized buffers used by the
        serializer associated with this stream.

        The capacity is absolute and does not do any
        accounting for any octets required by a chunked
        transfer encoding.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    capacity() const noexcept;

    /** Returns the number of octets serialized by this
        stream.

        The associated serializer stores stream output in its
        internal buffers. The stream returns the size of this
        output.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    size() const noexcept;

    /** Return true if the stream cannot currently hold
        additional output data.

        The fixed-sized buffers maintained by the associated
        serializer can be sufficiently full from previous
        calls to @ref stream::commit.

        This function can be called to determine if the caller
        should drain the serializer via @ref serializer::consume calls
        before attempting to fill the buffer sequence
        returned from @ref stream::prepare.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    is_full() const noexcept;

    /** Returns a MutableBufferSequence for storing
        serializer input. If `n` bytes are written to the
        buffer sequence, @ref stream::commit must be called
        with `n` to update the backing serializer's buffers.

        The returned buffer sequence is as wide as is
        possible.

        @exception std::length_error Thrown if the stream
        has insufficient capacity and a chunked transfer
        encoding is being used
    */
    BOOST_HTTP_PROTO_DECL
    buffers_type
    prepare() const;

    /** Make `n` bytes available to the serializer.

        Once the buffer sequence returned from @ref stream::prepare
        has been filled, the input can be marked as ready
        for serialization by using this function.

        @exception std::logic_error Thrown if `commit` is
        called with 0.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(std::size_t n) const;

    /** Indicate that no more data is coming and that the
        body should be treated as complete.

        @excpeption std::logic_error Thrown if the stream
        has been previously closed.
    */
    BOOST_HTTP_PROTO_DECL
    void
    close() const;

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

//---------------------------------------------------------

template<
    class ConstBufferSequence,
    class>
void
serializer::
start(
    message_view_base const& m,
    ConstBufferSequence&& body)
{
    start_init(m);
    auto const& bs =
        ws_.emplace<ConstBufferSequence>(
            std::forward<ConstBufferSequence>(body));

    std::size_t n = std::distance(
        buffers::begin(bs),
        buffers::end(bs));

    buf_ = make_array(n);
    auto p = buf_.data();
    for(buffers::const_buffer b : buffers::range(bs))
        *p++ = b;

    start_buffers(m);
}

template<
    class Source,
    class... Args,
    class>
Source&
serializer::
start(
    message_view_base const& m,
    Args&&... args)
{
    static_assert(
        !std::is_abstract<Source>::value, "");
    static_assert(
        std::is_constructible<Source, Args...>::value ||
        std::is_constructible<Source, detail::workspace&, Args...>::value,
        "The Source cannot be constructed with the given arguments");

    start_init(m);
    auto& src = construct_source<Source>(
        std::forward<Args>(args)...);
    start_source(m, std::addressof(src));
    return src;
}

//------------------------------------------------

inline
auto
serializer::
make_array(std::size_t n) ->
    detail::array_of_const_buffers
{
    return {
        ws_.push_array(n,
        buffers::const_buffer{}),
        n };
}

} // http_proto
} // boost

#endif
