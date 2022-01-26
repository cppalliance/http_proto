//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_TYPE_INDEX_HPP
#define BOOST_HTTP_PROTO_DETAIL_TYPE_INDEX_HPP

namespace boost {
namespace http_proto {
namespace detail {

// like std::type_index,
// but without requiring RTTI
using type_index = void const*;

// return type_index for T.
// This might not be unique in
// a shared-library scenario.
template <class T>
type_index
get_type_index() noexcept
{
    static constexpr char c{};
    return &c;
}

} // detail
} // http_proto
} // boost

#endif
