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
#include <boost/http_proto/forward_range.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

#if 0
class iterator_impl
{
public:
    string_view value;
    char const* end;

    iterator_impl(
        iterator_impl const&) = default;
    iterator_impl&
        operator=(iterator_impl const&) = default;

    iterator_impl()
};
#endif

struct token_list_type
{
    struct state
    {
        string_view value;
    };

    BOOST_HTTP_PROTO_DECL
    static
    char const*
    begin(
        state& st,
        char const* start,
        char const* end,
        error_code& ec) noexcept;

    BOOST_HTTP_PROTO_DECL
    static
    char const*
    increment(
        state& st,
        char const* start,
        char const* end,
        error_code& ec) noexcept;

private:
    static
    inline
    char const*
    skip_comma_ows_seq(
        char const* it,
        char const* end) noexcept;
};

using token_list = forward_range<token_list_type>;

} // http_proto
} // boost

#endif
