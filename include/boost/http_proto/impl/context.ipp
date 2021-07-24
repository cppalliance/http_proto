//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CONTEXT_IPP
#define BOOST_HTTP_PROTO_IMPL_CONTEXT_IPP

#include <boost/http_proto/context.hpp>
#include <vector>

namespace boost {
namespace http_proto {

struct context::data
{
    std::vector<std::unique_ptr<
        service>> vsp;
};

context::
~context()
{
}

context::
context() noexcept
    : p_(new data)
{
}

void
context::
insert_service(
    std::unique_ptr<service> sp)
{
    p_->vsp.emplace_back(std::move(sp));
}

} // http_proto
} // boost

#endif
