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

class serializer
{
    context& ctx_;
    char* buf_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t size_ = 0;
    string_view hs_;
    string_view bs_;
    header_info hi_;

public:
    /** Constructor (deleted)
    */
    serializer(
        serializer const&) = delete;

    /** Assignment (deleted)
    */
    serializer& operator=(
        serializer const&) = delete;

    /** Destructor

        All dynamically allocated memory
        is released upon destruction.
    */
    BOOST_HTTP_PROTO_DECL
    ~serializer();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    serializer(
        context& ctx) noexcept;

    /** Return true if serialization is complete
    */
    bool
    is_complete() const noexcept
    {
        return true;
    }

    /** Reset the serializer for a new message

        The current header, body, and encoding
        if present will be reset.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset() noexcept;

    /** Set the header for the current message
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_header(
        header_info const& hi) noexcept;

    /** Set the body for the current message
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_body(
        void const* data,
        std::size_t size) noexcept;

    /** Clear the contents without affecting the capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Return the next set of output buffers
    */
    BOOST_HTTP_PROTO_DECL
    const_buffer_pair
    prepare(error_code& ec);

    /** Consume bytes in the output buffers
    */
    void
    consume(std::size_t n) noexcept;
};

//------------------------------------------------

struct buffers{};

class serializer_
{
    char* buf_ = nullptr;
    std::size_t cap_ = 0;

    string_view hs_; // header
    string_view bs_; // body

public:
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    reserve(std::size_t bytes);

    BOOST_HTTP_PROTO_DECL
    void
    reset(header_info const& hi);

    BOOST_HTTP_PROTO_DECL
    void
    reset_for_head(
        header_info const& hi);

    BOOST_HTTP_PROTO_DECL
    bool
    is_complete() const noexcept;

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
    buffers
    prepare(error_code& ec);

    BOOST_HTTP_PROTO_DECL
    buffers
    prepare(
        std::size_t bytes,
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    string_view
    peek_output() const noexcept;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif
