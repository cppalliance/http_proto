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
#endif

/** A serializer for HTTP/1 messages

    This is used to serialize one or more complete
    HTTP/1 messages. Each message consists of a
    required header followed by an optional body.
*/
class BOOST_SYMBOL_VISIBLE
    serializer
{
public:
    /** A ConstBuffers representing the output
    */
    class const_buffers_type;

    struct stream;

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

    static
    constexpr
    std::size_t
    chunked_overhead_ =
        16 +        // size
        2 +         // CRLF
        2 +         // CRLF
        1 +         // "0"
        2 +         // CRLF
        2;          // CRLF

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
};

//------------------------------------------------

struct serializer::stream
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
        buffers::mutable_buffer_pair;

    BOOST_HTTP_PROTO_DECL
    std::size_t
    capacity() const;

    BOOST_HTTP_PROTO_DECL
    std::size_t
    size() const;

    BOOST_HTTP_PROTO_DECL
    buffers_type
    prepare(std::size_t n) const;

    BOOST_HTTP_PROTO_DECL
    void
    commit(std::size_t n) const;

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
