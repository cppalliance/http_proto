//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/detail/file_stdio.hpp>

#include "file_test.hpp"

namespace boost {
namespace http_proto {
namespace detail {

class file_stdio_test
{
public:
    void
    run()
    {
#ifdef _WIN32
        test_file<file_stdio, true>();
#else
        test_file<file_stdio>();
#endif
    }
};

TEST_SUITE(
    file_stdio_test,
    "boost.http_proto.detail.file_stdio");

} // detail
} // http_proto
} // boost
