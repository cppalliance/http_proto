//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/context.hpp>

#include <boost/http_proto/decoder.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class test_service
    : public context::service
{
public:
    struct test_decoder_type
        : decoder_type
    {
        std::unique_ptr<decoder>
        make_decoder() override
        {
            return nullptr;
        }
    };

    test_decoder_type td;

    explicit
    test_service(
        context& ctx)
    {
        ctx.add_content_decoder(
            "test-content-decoder", td);

        ctx.add_transfer_decoder(
            "test-transfer-decoder", td);
    }
};

class context_test
{
public:
    void
    testContext()
    {
        // default construction
        {
            context ctx;
        }
    }

    void
    testDecoders()
    {
        context ctx;
        make_service<test_service>(ctx);
    }

    void
    run()
    {
        testContext();
        testDecoders();
    }
};

TEST_SUITE(context_test, "boost.http_proto.context");

} // http_proto
} // boost
