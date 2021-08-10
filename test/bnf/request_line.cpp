//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/request_line.hpp>

#include "test_suite.hpp"
#include "test_bnf.hpp"

namespace boost {
namespace http_proto {

class request_line_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(request_line_test, "boost.http_proto.request_line");

} // http_proto
} // boost
