//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/response_view.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class response_view_test
{
public:
    void
    run()
    {
        // response_view()
        {
            response_view req;
        }

        // response_view(response_view const&)
        {
            {
                response_view res1;
                response_view res2(res1);
                (void)res2;
            }
        }

        // operator=(response_view const&)
        {
            {
                response_view res1;
                response_view res2;
                res1 = res2;
            }
        }
    }
};

TEST_SUITE(
    response_view_test,
    "boost.http_proto.response_view");

} // http_proto
} // boost

