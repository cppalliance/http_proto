//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/field_rule.hpp>

#include <boost/url/grammar/parse.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct field_rule_test
{
    void
    check(
        string_view s,
        string_view name,
        string_view value)
    {
        error_code ec;
        field_rule t;
        grammar::parse_string(s, ec, t);
        if( ec == grammar::error::leftover)
            ec = {};
        if(! BOOST_TEST(! ec))
            return;
        BOOST_TEST(t.v.name == name);
        BOOST_TEST(t.v.value == value);
    }

    void
    testFieldRule()
    {
        check("x:\r\n\r\n", "x", "");
        check("x: \r\n\r\n", "x", "");
        check("x:y\r\n\r\n", "x", "y");
        check("x: y\r\n\r\n", "x", "y");
        check("x:y \r\n\r\n", "x", "y");
        check("x: y \r\n\r\n", "x", "y");
        check("x: yy \r\n\r\n", "x", "yy");
        check("x: y y \r\n\r\n", "x", "y y");
        check("x: y \t y \r\n\r\n", "x", "y \t y");
        check("x: y \r\n \r\n\r\n", "x", "y");

        check("xy:\r\n\r\n", "xy", "");
        check("xyz:\r\n\r\n", "xyz", "");

        // obs-fold
        check("x:\r\n y\r\n\r\n", "x", "y");
    }

    void
    testReplaceObsFold()
    {
        auto const check =
            [](string_view sv0, string_view sv1)
        {
            std::string s(
                sv0.data(), sv0.size());
            replace_obs_fold(
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
        testFieldRule();
        testReplaceObsFold();
    }
};

TEST_SUITE(
    field_rule_test,
    "boost.http_proto.field_rule");

} // http_proto
} // boost
