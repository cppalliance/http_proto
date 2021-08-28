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

#include <boost/http_proto/codec/codecs.hpp>
#include <boost/http_proto/codec/decoder.hpp>

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
        ctx.codecs().add_decoder(
            "test-decoder", td);
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
        auto& ts = make_service<
            test_service>(ctx);
        BOOST_TEST(ctx.codecs().find_decoder("test-decoder") == &ts.td);
    }

    void
    testEncoders()
    {
    }

    void
    run()
    {
        testContext();
        testDecoders();
        testEncoders();
    }
};

TEST_SUITE(context_test, "boost.http_proto.context");

} // http_proto
} // boost
