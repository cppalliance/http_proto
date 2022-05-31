//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_HEADER_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_HEADER_IPP

#include <boost/http_proto/detail/header.hpp>
#include <utility>

namespace boost {
namespace http_proto {
namespace detail {

void
header::
swap(header& h) noexcept
{
    std::swap(cbuf, h.cbuf);
    std::swap(buf, h.buf);
    std::swap(cap, h.cap);
    std::swap(size, h.size);
    std::swap(count, h.count);
    std::swap(prefix, h.prefix);
    switch(this->kind)
    {
    case detail::kind::fields:
        break;
    case detail::kind::request:
        std::swap(
            req.method_len, h.req.method_len);
        std::swap(
            req.target_len, h.req.target_len);
        std::swap(req.method, h.req.method);
        std::swap(req.version, h.req.version);
        break;
    case detail::kind::response:
        std::swap(
            res.status_int, h.res.status_int);
        std::swap(res.status, h.res.status);
        std::swap(res.version, h.res.version);
        break;
    }
}

} // detail
} // http_proto
} // boost

#endif
