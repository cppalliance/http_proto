//
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SRC_ZLIB_SERVICE_HPP
#define BOOST_HTTP_PROTO_SRC_ZLIB_SERVICE_HPP

#include <boost/http_proto/service/service.hpp>
#include <boost/http_proto/service/zlib_service.hpp>
#include <boost/http_proto/detail/workspace.hpp>

namespace boost {
namespace http_proto {
namespace zlib {
namespace detail {

struct BOOST_HTTP_PROTO_ZLIB_DECL
    deflate_decoder_service
    : service
{
    struct config : decoder_config
    {
    };

    virtual
    config const&
    get_config() const noexcept = 0;

    virtual
    std::size_t
    space_needed() const noexcept = 0;

    virtual
    filter&
    make_deflate_filter(
        http_proto::detail::workspace& ws) const = 0;

    virtual
    filter&
    make_gzip_filter(
        http_proto::detail::workspace& ws) const = 0;
};

} // detail
} // zlib
} // http_proto
} // boost

#endif // BOOST_HTTP_PROTO_SRC_ZLIB_SERVICE_HPP
