//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SOURCE_IPP
#define BOOST_HTTP_PROTO_IMPL_SOURCE_IPP

#include <boost/http_proto/source.hpp>

namespace boost {
namespace http_proto {

source::
reserve_fn::
~reserve_fn() = default;

source::
~source() = default;

void
source::
maybe_reserve(
    std::size_t,
    reserve_fn const&)
{
}

} // http_proto
} // boost

#endif
