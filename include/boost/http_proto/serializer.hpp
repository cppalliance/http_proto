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
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/filter.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class request;
class response;
class request_view;
class response_view;
#endif

/** A serializer for HTTP/1 messages

    This is used to serialize one or more complete
    HTTP/1 messages. Each message consists of a
    required header followed by an optional body.
*/
class BOOST_SYMBOL_VISIBLE
    serializer
{
    char* buf_;
    std::size_t cap_;
    detail::header const* h_ = nullptr;
    detail::header h_copy_;
    asio::const_buffer cb_;
    source* ps_ = nullptr;

public:
    using const_buffers_type =
        const_buffers;

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    explicit
    serializer(
        std::size_t buffer_size);

    BOOST_HTTP_PROTO_DECL
    void
    reset() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    set_header(
        request_view const& req);

    BOOST_HTTP_PROTO_DECL
    void
    set_header(request const& req);

    BOOST_HTTP_PROTO_DECL
    void
    set_header(
        response_view const& res);

    BOOST_HTTP_PROTO_DECL
    void
    set_header(
        response const& res);

    template<
        class Body,
        class... Args>
    friend
    Body&
    set_body(
        serializer& sr,
        Args&&... args);

    BOOST_HTTP_PROTO_DECL
    bool
    is_complete() const noexcept;

    BOOST_HTTP_PROTO_DECL
    const_buffers
    prepare(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n) noexcept;

private:
    void set_header_impl(detail::header const& h);
    void set_header_impl(detail::header const* ph);

    template<class Body, class... Args>
    Body& set_body_impl(Args&&...);
};

//------------------------------------------------

template<
    class Body,
    class... Args>
Body&
set_body(
    serializer& sr,
    Args&&... args);

void
set_body(
    serializer& sr,
    string_view s);

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
