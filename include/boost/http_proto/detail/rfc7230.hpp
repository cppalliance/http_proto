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

#include <cstdint>

namespace boost {
namespace http_proto {
namespace detail {

static char is_token_char(char c) noexcept;
static bool is_pathchar(char c) noexcept;

#if 0
static bool is_digit(char c) noexcept;
static char is_alpha(char c) noexcept;
static char is_text(char c) noexcept;
static char is_qdchar(char c) noexcept;
static char is_qpchar(char c) noexcept;

// VFALCO TODO Make this return unsigned?
static std::int8_t unhex(char c) noexcept;
#endif

// converts to lower case,
// returns 0 if not a valid text char
//
static char to_value_char(char c) noexcept;

} // detail
} // http_proto
} // boost

#endif

