//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/deflate_service.hpp>

#include <boost/http_proto/context.hpp>
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class deflate_codec_test
{
public:
    void
    testDecoder()
    {
        context ctx;
        install_deflate_decoder(ctx);
    }

    void
    testEncoder()
    {
        context ctx;
        install_deflate_encoder(ctx);
    }

    void
    run()
    {
        testDecoder();
        testEncoder();
    }
};

TEST_SUITE(deflate_codec_test, "boost.http_proto.deflate_service");

} // http_proto
} // boost
