//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/digits_rule.hpp>

#include <boost/url/grammar/parse.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct digits_rule_test
{
    void
    invalid(
        string_view s)
    {
        error_code ec;
        digits_rule t;
        grammar::parse_string(
            s, ec, t);
        BOOST_TEST(ec.failed());
    }

    void
    valid(
        string_view s,
        std::uint64_t v)
    {
        error_code ec;
        digits_rule t;
        grammar::parse_string(
            s, ec, t);
        BOOST_TEST(! ec.failed());
        BOOST_TEST(! t.overflow);
        BOOST_TEST(t.s == s);
        BOOST_TEST(t.v == v);
    }

    void
    overflow(
        string_view s)
    {
        error_code ec;
        digits_rule t;
        grammar::parse_string(
            s, ec, t);
        BOOST_TEST(! ec.failed());
        BOOST_TEST(t.overflow);
        BOOST_TEST(t.s == s);
    }

    void
    run()
    {
        invalid("");
        invalid(" ");
        invalid("x");
        invalid("0 ");
        invalid(" 0");
        invalid("0x");
        invalid("123x");
        invalid("-1");
        invalid("+1");

        valid("0", 0);
        valid("00", 0);
        valid("1", 1);
        valid("123456789", 123456789);
        valid(
            "18446744073709551615",
             18446744073709551615ULL);
        valid(
            "000000000000000000000000000000"
            "18446744073709551615",
             18446744073709551615ULL);
        valid(
            "018446744073709551615",
              18446744073709551615ULL);
        valid("001", 1);

        overflow("18446744073709551616");
        overflow("18446744073709551620");
        overflow("018446744073709551620");
        overflow("018446744073709551620");
        overflow(
            "000000000000000000000000000000"
            "18446744073709551616");
    }
};

TEST_SUITE(
    digits_rule_test,
    "boost.http_proto.digits_rule");

} // http_proto
} // boost
