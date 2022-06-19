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

template<
    class Body,
    class... Args>
Body&
serializer::
set_body_impl(
    Args&&... args)
{
    // Body must be derived from source
    BOOST_STATIC_ASSERT(
        std::is_base_of<source, Body>::value);

    auto const Align = alignof(Body);
    auto const buf = reinterpret_cast<
        std::uintptr_t>(buf_);
    auto p = (buf + cap_ - sizeof(Body)
        ) & ~(Align - 1);
    if(p < buf)
        detail::throw_length_error(
            "set_body_impl",
            BOOST_CURRENT_LOCATION);
    auto& body = *new(reinterpret_cast<void*>(p)
        ) Body(std::forward<Args>(args)...);
    ps_ = &body;
    return body;
}

//------------------------------------------------

template<
    class Body,
    class... Args>
Body&
set_body(
    serializer& sr,
    Args&&... args)
{
    return
        sr.template set_body_impl<Body>(
            std::forward<Args>(args)...);
}

} // http_proto
} // boost

#endif
