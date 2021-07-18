//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_STRING_VIEW_HPP
#define BOOST_HTTP_PROTO_DETAIL_STRING_VIEW_HPP

namespace boost {
namespace http_proto {
namespace detail {

// Pulling in the UDL directly breaks in some places on MSVC,
// so introduce a namespace for this purprose.
namespace string_literals {
inline
string_view
operator"" _sv(char const* p, std::size_t n)
{
    return string_view{p, n};
}
} // string_literals

} // detail
} // http_proto
} // boost

#endif
