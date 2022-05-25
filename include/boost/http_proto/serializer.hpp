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
#include <boost/http_proto/header_info.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
class fields_view;
#endif

class BOOST_SYMBOL_VISIBLE
    serializer
{
    char* buf_ = nullptr;
    std::size_t cap_ = 0;

    string_view hs_; // header
    string_view bs_; // body

    const_buffer v_[2];

public:
    using const_buffers_type =
        const_buffers;

    using mutable_buffers_type =
        mutable_buffers;

    BOOST_HTTP_PROTO_DECL
    bool
    is_complete() const noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    reset(header_info const& hi);

    BOOST_HTTP_PROTO_DECL
    const_buffers_type
    prepare(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    commit(std::size_t n);

    BOOST_HTTP_PROTO_DECL
    const_buffers_type
    data() const noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t n);

#if 0
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    reserve(std::size_t bytes);

    BOOST_HTTP_PROTO_DECL
    void
    reset_for_head(
        header_info const& hi);

    BOOST_HTTP_PROTO_DECL
    void
    attach_body(string_view body);

    BOOST_HTTP_PROTO_DECL
    void
    attach_extensions(
        string_view extensions);

    BOOST_HTTP_PROTO_DECL
    void
    attach_end_of_body();

    BOOST_HTTP_PROTO_DECL
    void
    attach_end_of_body(
        fields_view const& trailers );

    BOOST_HTTP_PROTO_DECL
    void
    flush(); // ?

    BOOST_HTTP_PROTO_DECL
    void
    consume(std::size_t bytes);

    BOOST_HTTP_PROTO_DECL
    const_buffers
    prepare(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    const_buffers
    prepare(
        std::size_t bytes,
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    string_view
    peek_output() const noexcept;
#endif
};

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
