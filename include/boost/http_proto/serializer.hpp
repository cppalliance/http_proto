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
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/source.hpp>

#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/range.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/rts/context_fwd.hpp>
#include <boost/system/result.hpp>

#include <numeric>
#include <type_traits>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class serializer_service;
class message_view_base;
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
class serializer
{
public:
    using const_buffers_type =
        buffers::const_buffer_span;

    /** Serializer configuration settings.
    */
    struct config
    {
        /** True if serializer can encode deflate Content-Encoding.

            The @ref zlib::deflate_service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_deflate_encoder = false;

        /** True if serializer can encode gzip Content-Encoding.

            The @ref zlib::deflate_service must already be
            installed thusly, or else an exception
            is thrown.
        */
        bool apply_gzip_encoder = false;

        /** Specifies the zlib compression level 0..9.

            A compression level of 1 provides the fastest speed,
            while level 9 offers the best compression. Level 0
            applies no compression at all.
        */
        int zlib_comp_level = 6;

        /** Specifies the zlib windows bits 9..15.

            The windows bits controls the size of the history
            buffer used when compressing data. Larger values
            produce better compression at the expense of
            greater memory usage.
        */
        int zlib_window_bits = 15;

        /** Specifies the zlib memory level 1..9.

            The memory level controls the amount of memory
            used for the internal compression state. Larger
            values use more memory, but are faster and
            produce smaller output.
        */
        int zlib_mem_level = 8;

        /** Minimum space for payload buffering.

            This cannot be zero.
        */
        std::size_t payload_buffer = 8192;

        /** Space to reserve for type-erasure.

            This space is used for the following
            purposes:

            @li Storing an instance of the user-provided
                @ref source objects.

            @li Storing an instance of the user-provided
                ConstBufferSequence.

        */
        std::size_t max_type_erase = 1024;
    };

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
        rts::context& ctx);

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

private:
    friend class serializer_service;
    class filter;
    class const_buf_gen_base;
    template<class>
    class const_buf_gen;

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

    enum class style
    {
        empty,
        buffers,
        source,
        stream
    };

    rts::context& ctx_;
    serializer_service& svc_;

    detail::workspace ws_;

    const_buf_gen_base* buf_gen_;
    filter* filter_;
    source* source_;

    buffers::circular_buffer cb0_;
    buffers::circular_buffer cb1_;
    detail::array_of_const_buffers prepped_;
    buffers::const_buffer tmp_;

    style st_;
    bool more_input_;
    bool is_done_;
    bool is_header_done_;
    bool is_chunked_;
    bool needs_exp100_continue_;
    bool filter_done_;
};

//------------------------------------------------

/** Install the serializer service.
*/
BOOST_HTTP_PROTO_DECL
void
install_serializer_service(
    rts::context& ctx,
    serializer::config const& cfg);

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

class serializer::const_buf_gen_base
{
public:
    // Returns the next non-empty buffer,
    // or an empty buffer if none remain.
    virtual
    buffers::const_buffer
    next() = 0;

    // Size of remaining buffers
    virtual
    std::size_t
    size() const = 0;

    // Count of remaining non-empty buffers
    virtual
    std::size_t
    count() const = 0;

    // Returns true when there is no buffer or
    // the remaining buffers are empty
    virtual
    bool
    is_empty() const = 0;
};

template<class ConstBufferSequence>
class serializer::const_buf_gen
    : public const_buf_gen_base
{
    using it_t = decltype(buffers::begin(
        std::declval<ConstBufferSequence>()));

    ConstBufferSequence cbs_;
    it_t current_;
public:
    using const_buffer =
        buffers::const_buffer;

    explicit
    const_buf_gen(ConstBufferSequence cbs)
        : cbs_(std::move(cbs))
        , current_(buffers::begin(cbs_))
    {
    }

    const_buffer
    next() override
    {
        while(current_ != buffers::end(cbs_))
        {
            const_buffer buf = *current_++;
            if(buf.size() != 0)
                return buf;
        }
        return {};
    }

    std::size_t
    size() const override
    {
        return std::accumulate(
            current_,
            buffers::end(cbs_),
            std::size_t{},
            [](std::size_t sum, const_buffer cb) {
                return sum + cb.size(); });
    }

    std::size_t
    count() const override
    {
        return std::count_if(
            current_,
            buffers::end(cbs_),
            [](const_buffer cb) {
                return cb.size() != 0; });
    }

    bool
    is_empty() const override
    {
        return std::all_of(
            current_,
            buffers::end(cbs_),
            [](const_buffer cb) {
                return cb.size() == 0; });
    }
};

//---------------------------------------------------------

template<
    class ConstBufferSequence,
    class>
void
serializer::
start(
    message_view_base const& m,
    ConstBufferSequence&& cbs)
{
    static_assert(buffers::is_const_buffer_sequence<
            ConstBufferSequence>::value,
        "ConstBufferSequence type requirements not met");

    start_init(m);
    buf_gen_ = std::addressof(
        ws_.emplace<const_buf_gen<typename
        std::decay<ConstBufferSequence>::type>>(
                std::forward<ConstBufferSequence>(cbs)));
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
    source_ = std::addressof(src);
    start_source(m);
    return src;
}

} // http_proto
} // boost

#endif
