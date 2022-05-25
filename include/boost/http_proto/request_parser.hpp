//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_PARSER_HPP
#define BOOST_HTTP_PROTO_REQUEST_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    request_parser
    : public parser
{
    http_proto::method method_;

    off_t method_len_;
    off_t target_len_;

public:
    struct result : result_base
    {
        request_view header;
    };

    BOOST_HTTP_PROTO_DECL
    request_parser(
        config const& cfg,
        std::size_t buffer_bytes);

    /** Return a reference to the parsed request header.
    */
    BOOST_HTTP_PROTO_DECL
    request_view
    get() const noexcept;

    result const&
    get_() const noexcept
    {
        return r_;
    }

private:
    result r_;

    char*
    parse_start_line(
        char*,
        char const*,
        error_code&) noexcept override;

    void
    finish_header(
        error_code& ec) override;
};

} // http_proto
} // boost

#endif
