//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
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

class request_parser
    : public parser
{
    http_proto::method method_;

    off_t method_len_;
    off_t target_len_;
    off_t start_len_;

public:
    BOOST_HTTP_PROTO_DECL
    explicit
    request_parser(
        context& ctx) noexcept;

    /** Return a reference to the parsed request header.
    */
    BOOST_HTTP_PROTO_DECL
    request_view
    get() const noexcept;

private:
    char*
    parse_start_line(
        char* first,
        char const* last,
        error_code& ec) override;

    void
    finish_header(
        error_code& ec) override;
};

} // http_proto
} // boost

#endif
