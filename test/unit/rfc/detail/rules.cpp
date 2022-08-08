//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
//#include <boost/http_proto/rfc/detail/rules.hpp>
#if 0
#include "rule_tests.hpp"

namespace boost {
namespace http_proto {
namespace detail {

struct rules_test
{
public:
    void
    testRequestLine()
    {
        auto const r = request_line_rule;

        ok(r, "GET / HTTP/1.1\r\n");
        ok(r, "POST / HTTP/1.0\r\n");
        bad(r, "", grammar::error::need_more);
        bad(r, "G", grammar::error::need_more);
        bad(r, "GET ", grammar::error::need_more);
        bad(r, "GET /", grammar::error::need_more);
        bad(r, "GET / ", grammar::error::need_more);
        bad(r, "GET / HTTP", grammar::error::need_more);
        bad(r, "GET / HTTP/1", grammar::error::need_more);
        bad(r, "GET / HTTP/1.", grammar::error::need_more);
        bad(r, "GET / HTTP/1.1", grammar::error::need_more);
        bad(r, "GET / HTTP/1.1\r", grammar::error::need_more);
    }

    void
    testFieldRule()
    {
        auto const check = [](
            string_view s,
            string_view name,
            string_view value)
        {
            auto it = s.data();
            auto const end = it + s.size();
            auto rv = grammar::parse(
                it, end, field_rule);
            if(BOOST_TEST(rv.has_value()))
            {
                BOOST_TEST(rv->name == name);
                BOOST_TEST(rv->value == value);
            }
        };

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
        testRequestLine();
        testFieldRule();
        testReplaceObsFold();
    }
};

TEST_SUITE(
    rules_test,
    "boost.http_proto.rules_test");

} // detail
} // http_proto
} // boost
#endif
