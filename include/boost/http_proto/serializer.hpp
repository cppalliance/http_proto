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
    class buffers;

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    BOOST_HTTP_PROTO_DECL
    explicit
    serializer(
        std::size_t buffer_size);

    bool
    is_complete() const noexcept
    {
        return st_ == state::done;
    }

    BOOST_HTTP_PROTO_DECL
    void
    reset(
        message_view_base const& m) noexcept;

    template<class Body>
    void
    set_body(Body&& body);

    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    auto
    prepare() ->
        result<buffers>;

    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n) noexcept;

private:
    enum class state
    {
        init,
        ok,
        done
    };

    template<class Body>
    void
    set_body_impl(
        Body&& body,
        std::true_type);

    template<class Body>
    void
    set_body_impl(
        Body&& body,
        std::false_type);

    void init_impl();

    detail::workspace ws_;

    detail::header const* h_ = nullptr;
    source* src_ = nullptr;
    const_buffer hbuf_;
    detail::circular_buffer buf_;
    state st_ = state::init;
    bool more_ = false;

    const_buffer* cb_ = nullptr;
    std::size_t cbn_ = 0;
    std::size_t cbi_ = 0;
};

//------------------------------------------------

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
