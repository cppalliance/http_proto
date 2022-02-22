//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/quoted_string_rule.hpp>

#include <boost/url/grammar/parse.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class quoted_string_rule_test
{
public:
    void
    bad(string_view s)
    {
        error_code ec;
        auto it = s.data();
        auto const end = it + s.size();
        quoted_string_rule t;
        BOOST_TEST(! grammar::parse(
            it, end, ec, t));
    }

    void
    good(string_view s)
    {
        error_code ec;
        auto it = s.data();
        auto const end = it + s.size();
        quoted_string_rule t;
        BOOST_TEST(grammar::parse(
            it, end, ec, t));
    }

    void
    testParse()
    {
        bad("");
        bad(" ");
        bad("\t");
        bad("x");
        bad("\"");
        bad(" \"\"");

        good("\"" "\"");
        good("\"" "x" "\"");
        good("\"" "\t" "\"");
        good("\"" " \x80" "\"");
    }

    void
    run()
    {
        testParse();
    }
};

TEST_SUITE(
    quoted_string_rule_test,
    "boost.http_proto.quoted_string_rule");

} // http_proto
} // boost
