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

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class context_test
{
public:
    void
    testContext()
    {
        {
            context ctx;
        }
    }

    void
    run()
    {
        testContext();
    }
};

TEST_SUITE(context_test, "boost.http_proto.context");

} // http_proto
} // boost
