//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/service/zlib_service.hpp>

#ifdef BOOST_HTTP_PROTO_HAS_ZLIB

#include <boost/http_proto/context.hpp>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

struct zlib_service_test
{
    void
    run()
    {
        context ctx;
        
        zlib::deflate_decoder_service::config cfg;
        cfg.install(ctx);
    }
};

TEST_SUITE(
    zlib_service_test,
    "boost.http_proto.zlib_service");

} // http_proto
} // boost

#endif
