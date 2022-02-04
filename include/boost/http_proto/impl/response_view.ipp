//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_RESPONSE_VIEW_IPP
#define BOOST_HTTP_PROTO_IMPL_RESPONSE_VIEW_IPP

#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/status.hpp>

namespace boost {
namespace http_proto {

response_view::
response_view() noexcept
    : fields_view(2)
    , status_(http_proto::status::ok)
    , version_(http_proto::version::http_1_1)
{
}

response_view::
response_view(
    response_view const&) noexcept = default;

response_view&
response_view::
operator=(
    response_view const&) noexcept = default;

} // http_proto
} // boost

#endif
