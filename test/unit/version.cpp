//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/version.hpp>
#include <sstream>
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class version_test
{
public:
    void
    check(version v, core::string_view s)
    {
        std::stringstream ss;
        ss << v;
        BOOST_TEST(ss.str() == s);
    }

    void
    run()
    {
        check(version::http_1_0, "HTTP/1.0");
        check(version::http_1_1, "HTTP/1.1");
    }
};

TEST_SUITE(
    version_test,
    "boost.http_proto.version");

} // http_proto
} // boost
