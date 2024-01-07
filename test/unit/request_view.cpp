//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_view.hpp>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

class request_view_test
{
public:
    void
    run()
    {
        // request_view()
        {
            request_view req;
        }

        // request_view(request_view const&)
        {
            {
                request_view req1;
                request_view req2(req1);
            }
        }

        // operator=(request_view const&)
        {
            {
                request_view req1;
                request_view req2;
                req1 = req2;
            }
        }
    }
};

TEST_SUITE(
    request_view_test,
    "boost.http_proto.request_view");

} // http_proto
} // boost

