//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_RFC7230_HPP
#define BOOST_HTTP_PROTO_DETAIL_RFC7230_HPP

#include <boost/http_proto/char_set.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {
namespace detail {

static bool is_pchar(char c) noexcept;

using pchar_set =
    char_set_function<&is_pchar>;

// *( "," OWS )
static
char const*
skip_opt_comma_ows(
    char const* start,
    char const* end) noexcept;

// *( OWS "," )
static
char const*
skip_opt_ows_comma(
    bool& comma,
    char const* start,
    char const* end) noexcept;

static
char const*
parse_u64(
    std::uint64_t& result,
    char const* start,
    char const* end) noexcept;

} // detail
} // http_proto
} // boost

#endif

