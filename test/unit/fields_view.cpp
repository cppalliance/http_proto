//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_view.hpp>

#include <boost/http_proto/field.hpp>

#include "test_helpers.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct fields_view_test
{
    void
    testSpecial()
    {
        // fields_view()
        {
            fields_view fv;
        }

        // fields_view(fields_view const&)
        {
            {
                fields_view f1;
                fields_view f2(f1);
                (void)f2;
            }
        }

        // operator=(fields_view const&)
        {
            {
                fields_view f1;
                fields_view f2;
                f1 = f2;
            }
        }
    }

    void
    run()
    {
        testSpecial();
    }
};

TEST_SUITE(
    fields_view_test,
    "boost.http_proto.fields_view");

} // http_proto
} // boost
