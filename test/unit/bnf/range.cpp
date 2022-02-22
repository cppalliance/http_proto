//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/range.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

class range_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(range_test, "boost.http_proto.range");

} // bnf
} // http_proto
} // boost
