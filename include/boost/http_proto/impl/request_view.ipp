//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_VIEW_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_VIEW_IPP

#include <boost/http_proto/request_view.hpp>

namespace boost {
namespace http_proto {

request_view::
request_view(
    request_view const&) noexcept = default;

request_view&
request_view::
operator=(request_view const&) noexcept = default;

} // http_proto
} // boost

#endif
