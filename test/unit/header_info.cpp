//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/header_info.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

/*

fields_view_base     header_base
             \        /
            request_view



   fields_view_base
          |
     fields_base    header_base
              \     /
              request

              
              
         fields_view_base
                 |
            fields_base
                 |
              fields
*/
class header_info_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(
    header_info_test,
    "boost.http_proto.header_info");

} // http_proto
} // boost
