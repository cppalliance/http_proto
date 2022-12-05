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
    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    BOOST_HTTP_PROTO_DECL
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
        class Body
#ifndef BOOST_HTTP_PROTO_DOCS
        , class = typename
            std::enable_if<std::is_base_of<
                source, Body>::value>::type
#endif
    >
    void
    set_body(Body&& body);

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
    auto
    prepare() ->
        result<const_buffers>;

    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n) noexcept;

private:
    void set_header_impl(detail::header const& h);
    void set_header_impl(detail::header const* ph);

    template<class Body, class... Args>
    Body& set_body_impl(Args&&...);

    detail::workspace ws_;
    detail::header const* h_ = nullptr;
    detail::header h_copy_;
    source* src_ = nullptr;
    const_buffer bs_[8];
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
