//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/buffer.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class buffer_test
{
public:
    void run()
    {
        {
            mutable_buffer mb;
            const_buffer cb(mb);
            mb += 1;
            cb = cb + 1;
            mb = 1 + mb;
        }
        {
            const_buffers cb1;
            const_buffers cb2(cb1);
            cb1 = cb2;
        }
    }
};

TEST_SUITE(
    buffer_test,
    "boost.http_proto.buffer");

} // http_proto
} // boost
