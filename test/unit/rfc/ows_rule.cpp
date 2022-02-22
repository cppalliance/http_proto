//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/ows_rule.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct ows_rule_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    ows_rule_test,
    "boost.http_proto.ows_rule");

} // http_proto
} // boost
