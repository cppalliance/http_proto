//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/token_rule.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class token_rule_test
{
public:
    void
    testParse()
    {
        auto const match = [](
            string_view s,
            string_view m)
        {
            auto it = s.data();
            auto const end = it + s.size();
            error_code ec;
            token_rule t;
            if(! parse(it, end, ec, t))
                BOOST_TEST(m.empty());
            else
                BOOST_TEST(*t == m);
        };

        match("", "");
        match("x", "x");
        match("=", "");
        match("xy", "xy");
        match("=x", "");
        match("x=", "x");
        match("==", "");
        match("xy=", "xy");
        match("===", "");
    }

    void
    run()
    {
        testParse();
    }
};

TEST_SUITE(
    token_rule_test,
    "boost.http_proto.token_rule");

} // http_proto
} // boost
