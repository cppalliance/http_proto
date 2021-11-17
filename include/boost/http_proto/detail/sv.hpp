//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_SV_HPP
#define BOOST_HTTP_PROTO_DETAIL_SV_HPP

#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {
namespace detail {

// Pulling in the UDL directly breaks in some places on MSVC,
// so introduce a namespace for this purprose.
namespace string_literals {
inline
string_view
operator"" _sv(
    char const* p,
    std::size_t n) noexcept
{
    return string_view(p, n);
}
} // string_literals

} // detail
} // http_proto
} // boost

#endif
