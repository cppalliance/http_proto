//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/transfer_parameter_list.hpp>

#include "test_suite.hpp"
#include <sstream>

namespace boost {
namespace http_proto {

class transfer_parameter_list_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(transfer_parameter_list_test, "boost.http_proto.transfer_parameter_list");

} // http_proto
} // boost
