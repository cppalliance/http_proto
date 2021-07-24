//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CONTEXT_HPP
#define BOOST_HTTP_PROTO_IMPL_CONTEXT_HPP

#include <utility>

namespace boost {
namespace http_proto {

template<class T, class... Args>
T&
make_service(
    context& ctx,
    Args&&... args)
{
    T* const t = new T(
        std::forward<Args>(args)...);
    insert_service(std::unique_ptr<
        context::service>(t));
    return *t;
}

} // http_proto
} // boost

#endif
