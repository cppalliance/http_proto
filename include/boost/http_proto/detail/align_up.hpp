//
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_ALIGN_UP_HPP
#define BOOST_HTTP_PROTO_DETAIL_ALIGN_UP_HPP

#include <cstddef>

namespace boost {
namespace http_proto {
namespace detail {

constexpr
inline
std::size_t
align_up(std::size_t s, std::size_t A)
{
  return A * (
        (s + A - 1) / A);
}

} // detail
} // http_proto
} // boost

#endif
