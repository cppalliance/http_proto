//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_WORKSPACE_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_WORKSPACE_IPP

#include <boost/http_proto/detail/workspace.hpp>

namespace boost {
namespace http_proto {
namespace detail {

workspace::
any::
~any() = default;

void
workspace::
clear() noexcept
{
    auto const end =
        reinterpret_cast<
            any const*>(end_);
    auto p =
        reinterpret_cast<
            any const*>(head_);
    while(p != end)
    {
        auto next = p->next;
        p->~any();
        p = next;
    }
    head_ = end_;
}

} // detail
} // http_proto
} // boost

#endif
