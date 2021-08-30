//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/status_line.hpp>

#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"
#include "test_bnf.hpp"

namespace boost {
namespace http_proto {
namespace bnf {
namespace test {

BOOST_STATIC_ASSERT(
    is_element<status_line>::value);

class status_line_test
{
public:
    void
    run()
    {
        test::bad<status_line>(
            "");
        test::bad<status_line>(
            "GET / HTTP/1.0\r\n");
        test::bad<status_line>(
            "HTTP/9.9 0 OK\r\n");
        test::good<status_line>(
            "HTTP/1.1 200 OK\r\n");
    }
};

TEST_SUITE(
    status_line_test,
    "boost.http_proto.status_line");

} // test
} // bnf
} // http_proto
} // boost
