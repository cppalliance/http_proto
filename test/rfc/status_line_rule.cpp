//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/status_line_rule.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class status_line_rule_test
{
public:
    void
    run()
    {
#if 0
        test::bad<status_line>(
            "");
        test::bad<status_line>(
            "GET / HTTP/1.0\r\n");
        test::bad<status_line>(
            "HTTP/9.9 0 OK\r\n");
        test::good<status_line>(
            "HTTP/1.1 200 OK\r\n");
#endif
    }
};

TEST_SUITE(
    status_line_rule_test,
    "boost.http_proto.status_line_rule");

} // http_proto
} // boost
