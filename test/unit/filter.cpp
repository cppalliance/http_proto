//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/filter.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct filter_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    filter_test,
    "boost.http.proto.filter");

} // http_proto
} // boost
