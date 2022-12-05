//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/file_body.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct file_body_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    file_body_test,
    "boost.http_proto.file_body");

} // http_proto
} // boost
