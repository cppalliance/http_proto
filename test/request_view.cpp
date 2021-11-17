//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_view.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class request_view_test
{
public:
    void
    run()
    {
        // default ctor
        {
            request_view req;
        }

        // copy ctor
        {
            request_view r1;
            request_view r2(r1);
        }

        // copy assign
        {
            request_view r1;
            request_view r2;
            r1 = r2;
        }
    }
};

TEST_SUITE(request_view_test, "boost.http_proto.request_view");

} // http_proto
} // boost

