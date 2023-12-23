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
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/detail/type_traits.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/flat_buffer.hpp>
#include <boost/buffers/mutable_buffer_pair.hpp>
#include <boost/buffers/mutable_buffer_span.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/buffers/any_dynamic_buffer.hpp>
#include <boost/url/grammar/error.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
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
        return st_ == state::complete;
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
            (   st_ == state::complete &&
                got_eof_);
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
    template<class Sink>
#ifndef BOOST_HTTP_PROTO_DOCS
    typename std::enable_if<
            is_sink<Sink>::value,
        typename std::decay<Sink>::type
            >::type&
#else
    typename std::decay<Sink>::type&
#endif
    set_body(Sink&& sink);

    /** Return the available body data and consume it.

        The buffer referenced by the string view
        will be invalidated if any member function
        of the parser is called.
    */
    BOOST_HTTP_PROTO_DECL
    const_buffers_type
    pull_some();

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
    bool is_plain() const noexcept;
    void on_headers(system::error_code&);
    BOOST_HTTP_PROTO_DECL void on_set_body();
    void init_dynamic(system::error_code&);

    static constexpr unsigned buffers_N = 8;

    enum class state
    {
        // order matters
        reset,
        start,
        header,
        body,
        set_body,
        complete
    };

    enum class how
    {
        in_place,
        dynamic,
        sink,
        pull
    };

    context& ctx_;
    parser_service& svc_;
    detail::workspace ws_;
    detail::header h_;
    std::uint64_t body_avail_;
    std::uint64_t body_total_;
    std::uint64_t payload_remain_;
    std::size_t nprepare_;

    buffers::flat_buffer fb_;
    buffers::circular_buffer cb0_;
    buffers::circular_buffer cb1_;
    buffers::circular_buffer* body_buf_;
    buffers::mutable_buffer_pair mbp_;
    buffers::any_dynamic_buffer* dyn_;
    filter* filt_;
    sink* sink_;

    state st_;
    how how_;
    bool got_eof_;
//    bool need_more_;
    bool head_response_;
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
