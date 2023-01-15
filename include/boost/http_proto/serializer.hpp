//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_SERIALIZER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/source.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/circular_buffer.hpp>
#include <boost/http_proto/detail/consuming_buffers.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class request;
class response;
class request_view;
class response_view;
class message_view_base;
struct brotli_decoder_t;
struct brotli_encoder_t;
struct deflate_decoder_t;
struct deflate_encoder_t;
struct gzip_decoder_t;
struct gzip_encoder_t;
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
    class output;

    /** A MutableBuffers representing the input
    */
    using input_buffers =
        mutable_buffers_pair;

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
    explicit
    serializer(
        std::size_t buffer_size);

    /** Constructor
    */
    template<class P0, class... Pn>
    serializer(
        std::size_t buffer_size,
        P0&& p0,
        Pn&&... pn);

    //--------------------------------------------

    /** Reset the serializer for a new message

        The message will not contain a body.
        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.
    */
    void
    reset(
        message_view_base const& m)
    {
        reset_empty_impl(m);
    }

    /** Reset the serializer for a new message

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
        class ConstBuffers
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename
            std::enable_if<
                is_const_buffers<
                    ConstBuffers>::value
                        >::type
#endif
    >
    void
    reset(
        message_view_base const& m,
        ConstBuffers&& body);    

    /** Reset the serializer for a new message

        Changing the contents of the message
        after calling this function and before
        @ref is_done returns `true` results in
        undefined behavior.
    */
    template<
        class Source
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename
            std::enable_if<
                is_source<Source
                    >::value>::type
#endif
    >
    auto
    reset(
        message_view_base const& m,
        Source&& body) ->
            typename std::decay<
                Source>::type&;

    //--------------------------------------------

    struct stream
    {
        stream() = default;
        stream(stream const&) = default;
        stream& operator=
            (stream const&) = default;

        using buffers_type =
            mutable_buffers_pair;

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

    struct reserve_nothing
    {
        void
        operator()(
            std::size_t,
            source::reserve_fn const&) noexcept
        {
        }
    };

    template<
        class MaybeReserve = reserve_nothing>
    stream
    reset_stream(
        message_view_base const& m,
        MaybeReserve&& maybe_reserve = {});

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
        result<output>;

    /** Consume bytes from the output area.
    */
    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n);

private:
    void apply_params() noexcept;

    template<
        class P0,
        class... Pn>
    void
    apply_params(
        P0&&, Pn&&...);

    template<class Param>
    void
    apply_param(
        Param const&) = delete;

    BOOST_HTTP_PROTO_DECL void
        apply_param(brotli_decoder_t const&);
    BOOST_HTTP_PROTO_DECL void
        apply_param(brotli_encoder_t const&);
    BOOST_HTTP_PROTO_DECL void
        apply_param(deflate_decoder_t const&);
    BOOST_HTTP_PROTO_DECL void
        apply_param(deflate_encoder_t const&);
    BOOST_HTTP_PROTO_DECL void
        apply_param(gzip_decoder_t const&);
    BOOST_HTTP_PROTO_DECL void
        apply_param(gzip_encoder_t const&);

    BOOST_HTTP_PROTO_DECL void
        do_reserve(source&, std::size_t);

    BOOST_HTTP_PROTO_DECL void
        reset_empty_impl(
        message_view_base const&);

    BOOST_HTTP_PROTO_DECL void
        reset_buffers_impl(
        message_view_base const&,
            const_buffer*, std::size_t);

    BOOST_HTTP_PROTO_DECL void
        reset_source_impl(
        message_view_base const&, source*);

    BOOST_HTTP_PROTO_DECL void
        reset_stream_impl(
        message_view_base const&, source&);

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

    class reserve;

    detail::workspace ws_;
    const_buffer*   hp_;  // header
    const_buffer*   pp_;
    std::size_t     pn_;
    source*         src_;

    detail::circular_buffer dat1_;
    detail::circular_buffer dat2_;

    style st_;
    bool more_;
    bool is_done_;
    bool is_chunked_;
    bool is_expect_continue_;
    bool is_reserving_ = false;
};

//------------------------------------------------

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
