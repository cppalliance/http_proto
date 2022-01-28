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
request_view() noexcept = default;

request_view::
request_view(
    request_view const&) noexcept = default;

request_view&
request_view::
operator=(request_view const&) noexcept = default;

request_view::
request_view(
    ctor_params const& init) noexcept
    : fields_view(init)
    , method_len_(static_cast<
        off_t>(init.method_len))
    , target_len_(static_cast<
        off_t>(init.target_len))
    , method_(init.method)
    , version_(init.version)
{
    BOOST_ASSERT(method_len_ <=
        BOOST_HTTP_PROTO_MAX_HEADER);
    BOOST_ASSERT(target_len_ <=
        BOOST_HTTP_PROTO_MAX_HEADER);
}

} // http_proto
} // boost

#endif
