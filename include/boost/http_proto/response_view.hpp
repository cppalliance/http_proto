//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP
#define BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
enum class status : unsigned short;
enum class version : char;
#endif

class BOOST_SYMBOL_VISIBLE
    response_view
    : public fields_view_base
{
    friend class response;
    friend class response_parser;

    http_proto::status status_;
    http_proto::version version_;

public:
    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response_view() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response_view(
        response_view const&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    response_view&
    operator=(
        response_view const&) noexcept;

    /** Return the reason string
    */
    string_view
    reason() const noexcept
    {
        return string_view(
            cbuf_ + 14,
            start_len_ - 16);
    }

    /** Return the status code
    */
    http_proto::status
    status() const noexcept
    {
        return status_;
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
