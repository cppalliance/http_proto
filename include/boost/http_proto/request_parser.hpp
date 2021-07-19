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
#include <boost/http_proto/basic_parser.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

class request_parser
    : public basic_parser
{
    http_proto::method method_;
    int version_;
    unsigned short n_method_;
    unsigned short n_target_;

public:
    BOOST_HTTP_PROTO_DECL
    request_parser() noexcept;

    BOOST_HTTP_PROTO_DECL
    request_view
    get() const noexcept;

private:
    BOOST_HTTP_PROTO_DECL
    void
    parse_start_line(
        char const*& in, char const* last,
        error_code& ec) override;

    static
    void
    parse_method(
        char const*& it, char const* last,
        string_view& result, error_code& ec);

    static
    void
    parse_target(
        char const*& it, char const* last,
        string_view& result, error_code& ec);

    BOOST_HTTP_PROTO_DECL
    void
    finish_header(
        error_code& ec) override;
};

} // http_proto
} // boost

#endif
