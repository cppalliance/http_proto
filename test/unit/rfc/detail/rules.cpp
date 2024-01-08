//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/detail/rules.hpp>

#include "test_rule.hpp"

namespace boost {
namespace http_proto {
namespace detail {

struct rules_test
{
    void
    testReplaceObsFold()
    {
        auto const check =
            []( core::string_view sv0,
                core::string_view sv1)
        {
            std::string s(
                sv0.data(), sv0.size());
            remove_obs_fold(
                &s[0], s.data() + s.size());
            BOOST_TEST(sv1 == s);
        };

        check("", "");
        check(" ", " ");
        check("\t", "\t");
        check("\r", "\r");
        check("\r\n", "\r\n");
        check(" \r\n", " \r\n");
        check("\r\n\r", "\r\n\r");
        check(" \r\n.", " \r\n.");
        check(" \r\n\t", "   \t");
        check("\r\n\r\n", "\r\n\r\n");
        check(".\r\n .\r\n", ".   .\r\n");
        check(" \r\n \r", "    \r");
        check(" \r\n \r ", "    \r ");
    }

    void
    run()
    {
        testReplaceObsFold();
    }
};

TEST_SUITE(
    rules_test,
    "boost.http_proto.rules_test");

} // detail
} // http_proto
} // boost
