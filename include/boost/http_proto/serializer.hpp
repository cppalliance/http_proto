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

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/source.hpp>
#include <boost/http_proto/detail/array_of_buffers.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/flat_buffer.hpp>
#include <boost/buffers/range.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/system/result.hpp>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include <zlib.h>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class request;
class response;
class request_view;
class response_view;
class message_view_base;
#endif

inline
void* zalloc_impl(
    void* /* opaque */,
    unsigned items,
    unsigned size)
{
    try {
        return ::operator new(items * size);
    } catch(std::bad_alloc const&) {
        return Z_NULL;
    }
}

inline
void zfree_impl(void* /* opaque */, void* addr)
{
    ::operator delete(addr);
}

struct zlib_filter
{
    z_stream stream_;
    detail::workspace ws_;
    buffers::circular_buffer buf_;
    content_coding_type coding_ = content_coding_type::none;
    bool is_done_ = false;

    zlib_filter()
        : ws_(1024), buf_(ws_.data(), ws_.size())
    {
        stream_.zalloc = &zalloc_impl;
        stream_.zfree = &zfree_impl;
        stream_.opaque = nullptr;
    }

    zlib_filter(zlib_filter const&) = delete;
    zlib_filter& operator=(zlib_filter const&) = delete;

    ~zlib_filter()
    {
        deflateEnd(&stream_);
    }

    void init()
    {
        int ret = -1;

        int window_bits = 15;
        if( coding_ == content_coding_type::gzip )
            window_bits += 16;

        int mem_level = 8;

        ret = deflateInit2(
            &stream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
            window_bits, mem_level, Z_DEFAULT_STRATEGY);

        if( ret != Z_OK )
            throw ret;

        stream_.next_out = nullptr;
        stream_.avail_out = 0;

        stream_.next_in = nullptr;
        stream_.avail_in = 0;
    }

    void reset(enum content_coding_type coding)
    {
        BOOST_ASSERT(coding != content_coding_type::none);
        if( coding_ == coding )
        {
            int ret = -1;
            ret = deflateReset(&stream_);
            if( ret != Z_OK )
                throw ret;
        }
        else
        {
            if( coding_ != content_coding_type::none )
                deflateEnd(&stream_);
            coding_ = coding;
            init();
        }
    }
};

/** A serializer for HTTP/1 messages

    This is used to serialize one or more complete
    HTTP/1 messages. Each message consists of a
    required header followed by an optional body.
*/
class BOOST_SYMBOL_VISIBLE
    serializer
{
public:
    class const_buffers_type;

    struct stream;

    zlib_filter* zlib_filter_ = nullptr;

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    serializer();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    serializer(
        serializer&&) noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    serializer(
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

    /** Create a new stream object associated with the
        serializer.

        The returned stream must not outlive its backing
        serializer object.

        Streams permit a user to supply input data to
        the serializer using a bounded sequence of mutable
        buffers.

        \code{.cpp}
        std::string msg = "Hello, world!";
        std::size_t buf_size = 16 * 1024;
        http_proto::serializer sr(buf_size);

        auto stream = sr.start_stream();
        auto bufs = stream.prepare();
        auto s = buffers::buffer_size(bufs);
        auto n = buffers::buffer_copy(
            bufs,
            buffers::make_buffer(
                msg.data(),
                std::min(s, msg.size())));
        stream.commit(n);

        auto cbs = sr.prepare().value();
        // `cbs` contains the serialized octets corresponding
        // to our `msg`
        \endcode
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
                buffered_base::allocator&,
                Args...>::value>::type* = nullptr>
    Source&
    construct_source(Args&&... args)
    {
        buffered_base::allocator a(
            ws_.data(),
            (ws_.size() - ws_.space_needed<Source>()) / 2,
            false);
        auto& src = ws_.emplace<Source>(
            a, std::forward<Args>(args)...);
        ws_.reserve_front(a.size_used());
        return src;
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
    source* src_;

    buffers::circular_buffer tmp0_;
    buffers::circular_buffer tmp1_;
    detail::array_of_const_buffers out_;

    buffers::const_buffer* hp_;  // header

    style st_;
    bool more_;
    bool is_done_;
    bool is_chunked_;
    bool is_expect_continue_;
    bool is_compressed_ = false;
};

//------------------------------------------------

/**
    A proxy type used to pass bounded input to the
    associated serializer.
*/
struct serializer::stream
{
    /** Default constructor.

        Creates a stream without an associated serializer
        object.
    */
    stream() = default;

    /** Copy constructor.

        The constructed stream will share the same
        serializer as `other`.
    */
    stream(stream const& other) = default;

    /** Assignment operator

        The current stream will share the same serializer
        as `other`.
    */
    stream& operator= (
        stream const& other) = default;

    /**
        A MutableBufferSequence consisting of a buffer pair.
     */
    using buffers_type =
        buffers::mutable_buffer_pair;

    /**
        Returns the remaining available capacity.

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

    /**
        Returns the number of octets serialized by this
        stream.

        The associated serializer stores stream output in its
        internal buffers. The stream returns the size of this
        output.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    size() const noexcept;

    /**
        Returns a boolean indicating if the stream can
        receive more input.

        The fixed-sized buffers maintained by the associated
        serializer can be sufficiently full from previous
        calls to \ref stream::commit.

        This function can be called to determine if the user
        should drain the serializer via \ref serializer::consume calls
        before attempting to fill the buffer sequence
        returned from \ref stream::prepare.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    is_full() const noexcept;

    /**
        Returns a MutableBufferSequence capable of storing
        input from the user

        The returned buffer sequence is as wide as is
        possible. If a non-chunked transfer encoding is
        being used than the returned sequence encompasses
        the unused area of the serializer's fixed-sized
        buffers.

        If a chunked transer encoding is used then space is
        reserved for the chunk header in addition to the
        closing CRLF required for the chunk data. In
        addition to this, space is also reserved for the
        last-chunk.

        This is done so that users can chain calls to
        \ref stream::commit and \ref stream::close
        without having to drain the serializer via
        \ref serializer::consume calls.

        \exception std::length_error Thrown if the stream
        has insufficient capacity and a chunked transfer
        encoding is being used
    */
    BOOST_HTTP_PROTO_DECL
    buffers_type
    prepare() const;

    /**
        Serialize and commit `n` bytes.

        Once the sequence returned from \ref prepare has been
        filled, the input can be serialized and committed to the
        associated serializer's output area via a call to `commit(n)`.

        If a chunked transfer encoding is being used then commit
        is responsible for writing the chunk-header and also the
        closing CRLF for the chunk-data. `n` denotes the size
        of the chunk.

        \exception std::logic_error Thrown if commit is
        called with 0. Instead, the closing chunk must be
        written by a call to \ref stream::close.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(std::size_t n) const;

    /**
        Close the stream.

        close() writes the last-chunk to the underlying buffers
        of the stream's associated serializer, i.e. `0\r\n\r\n`.

        \excpeption std::logic_error Thrown if the stream
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

/** A ConstBufferSequence representing the output
*/
class serializer::
    const_buffers_type
{
    std::size_t n_ = 0;
    buffers::const_buffer const* p_ = nullptr;

    friend class serializer;

    const_buffers_type(
        buffers::const_buffer const* p,
        std::size_t n) noexcept
        : n_(n)
        , p_(p)
    {
    }

public:
    using iterator = buffers::const_buffer const*;
    using const_iterator = iterator;
    using value_type = buffers::const_buffer;
    using reference = buffers::const_buffer;
    using const_reference = buffers::const_buffer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    const_buffers_type() = default;
    const_buffers_type(
        const_buffers_type const&) = default;
    const_buffers_type& operator=(
        const_buffers_type const&) = default;

    iterator
    begin() const noexcept
    {
        return p_;
    }

    iterator
    end() const noexcept
    {
        return p_ + n_;
    }
};

//------------------------------------------------

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
    for(buffers::const_buffer b :
            buffers::range(bs))
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
        std::is_constructible<Source, Args...>::value ||
        std::is_constructible<Source, buffered_base::allocator&, Args...>::value,
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
