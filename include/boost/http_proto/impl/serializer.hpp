//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP

#include <boost/http_proto/detail/except.hpp>
#include <new>
#include <utility>

namespace boost {
namespace http_proto {

template<class Body, class>
void
serializer::
set_body(Body&& body)
{
    set_body_impl<typename
        std::decay<Body>::type>(
            std::forward<Body>(body));
}

template<
    class Source,
    class... Args>
Source&
serializer::
set_body_impl(
    Args&&... args)
{
    static_assert(
        std::is_base_of<
            source, Source>::value,
        "Type requirements not met");
    auto& src = detail::push<Source>(ws_,
        std::forward<Args>(args)...);
    src_ = &src;
    return src;
}

//------------------------------------------------

template<
    class Source,
    class... Args>
Source&
set_body(
    serializer& sr,
    Args&&... args)
{
    return
        sr.template set_body_impl<Source>(
            std::forward<Args>(args)...);
}

} // http_proto
} // boost

#endif
