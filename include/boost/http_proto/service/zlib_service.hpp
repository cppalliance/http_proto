//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_ZLIB_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_ZLIB_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/filter.hpp>
#include <boost/http_proto/service/service.hpp>
#include <boost/http_proto/detail/workspace.hpp>

namespace boost {
namespace http_proto {
namespace zlib {

struct decoder_config
{
    unsigned max_window_bits = 15;
};

//------------------------------------------------

struct deflate_decoder_service
    : service
{
    struct config : decoder_config
    {
        BOOST_HTTP_PROTO_ZLIB_DECL
        void
        install(context& ctx);
    };

    virtual
    config const&
    get_config() const noexcept = 0;

    virtual
    std::size_t
    space_needed() const noexcept = 0;

    virtual
    filter&
    make_filter(detail::workspace& ws) const = 0;
};

//------------------------------------------------

} // zlib
} // http_proto
} // boost

#endif
