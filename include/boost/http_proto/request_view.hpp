//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_VIEW_HPP
#define BOOST_HTTP_PROTO_REQUEST_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
enum class method : char;
enum class version : char;
#endif

class BOOST_SYMBOL_VISIBLE
    request_view
    : public fields_view
{
    friend class request;
    friend class request_parser;

    off_t method_len_ = 0;
    off_t target_len_ = 0;
    http_proto::method method_;
    http_proto::version version_;

    struct ctor_params
        : fields_view::ctor_params
    {
        std::size_t method_len;
        std::size_t target_len;
        http_proto::method method;
        http_proto::version version;
    };

    BOOST_HTTP_PROTO_DECL
    explicit
    request_view(
        ctor_params const& init) noexcept;

public:
    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request_view(
        request_view const&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    request_view&
    operator=(request_view const&) noexcept;

    /** Constructor

        The contents of default constructed
        messages are undefined. The only valid
        operations are assignment and destruction.
    */
    BOOST_HTTP_PROTO_DECL
    request_view() noexcept;

    /** Return the known method constant
    */
    http_proto::method
    method() const noexcept
    {
        return method_;
    };

    /** Return the exact method string
    */
    string_view
    method_str() const noexcept
    {
        return string_view(
            base_, method_len_);
    }

    /** Return the request-target string
    */
    string_view
    target() const noexcept
    {
        return string_view(
            base_ +
                method_len_ + 1,
            target_len_);
    }

    /** Return the HTTP-version
    */
    http_proto::version
    version() const noexcept
    {
        return version_;
    }
};

} // http_proto
} // boost

#endif
