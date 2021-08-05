//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_TOKEN_LIST_HPP
#define BOOST_HTTP_PROTO_TOKEN_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf_range.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

struct token_list_bnf
{
    string_view value;

    BOOST_HTTP_PROTO_DECL
    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec) noexcept;

    BOOST_HTTP_PROTO_DECL
    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec) noexcept;

private:
    bool comma_;

    static
    char const*
    skip_opt_comma_ows(
        char const* start,
        char const* end) noexcept;

    char const*
    skip_opt_ows_comma(
        char const* start,
        char const* end) noexcept;

    static
    char const*
    skip_token(
        char const* start,
        char const* end) noexcept;
};

using token_list = bnf_range<token_list_bnf>;

} // http_proto
} // boost

#endif
