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
    class output_buffers;
    using input_buffers =
        mutable_buffers_pair;

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    serializer(
        std::size_t buffer_size);

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
        result<output_buffers>;

    /** Consume bytes from the output area.
    */
    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n) noexcept;

    /** Return the input area.
    */
    BOOST_HTTP_PROTO_DECL
    input_buffers
    data() noexcept;

    /** Commit bytes to the input area.
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(
        std::size_t bytes,
        bool end);

    //--------------------------------------------

    /** Reset the serializer for a new message
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset(
        message_view_base const& m);

    /** Reset the serializer for a new message

        The message will not contain a body.
    */
    template<class Body>
    void
    reset(
        message_view_base const& m,
        Body&& body);

private:
    BOOST_HTTP_PROTO_DECL
    void
    reset_impl(
        message_view_base const& m);

    template<class Source>
    void
    reset_impl(
        message_view_base const& m,
        Source&& source,
        std::true_type);

    template<class Buffers>
    void
    reset_impl(
        message_view_base const& m,
        Buffers&& buffers,
        std::false_type);

    detail::workspace ws_;
    const_buffer hbuf_;
    const_buffer* cb_ = nullptr;
    std::size_t cbn_ = 0;
    bool is_done_;
    bool is_expect_continue_;

    source* src_ = nullptr;
    detail::circular_buffer buf_;
    bool more_ = false;
};

//------------------------------------------------

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
