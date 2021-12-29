//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/number.hpp>

#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"
#include "test_rule.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(
    is_element<dec_number>::value);

BOOST_STATIC_ASSERT(
    is_element<hex_number>::value);

class number_test
{
public:
    void
    good_dec(
        string_view s,
        std::uint64_t v)
    {
        dec_number p;
        error_code ec;
        auto const end =
            s.data() + s.size();
        auto it = p.parse(
            s.data(), end, ec);
        BOOST_TEST(! ec);
        if(! ec)
        {
            BOOST_TEST(p.value() == v);
            BOOST_TEST(it == end);
        }
    }


    void
    bad_dec(string_view s)
    {
        dec_number p;
        error_code ec;
        auto const end =
            s.data() + s.size();
        p.parse(s.data(), end, ec);
        BOOST_TEST(ec.failed());
    }

    void
    good_hex(
        string_view s,
        std::uint64_t v)
    {
        hex_number p;
        error_code ec;
        auto const end =
            s.data() + s.size();
        auto it = p.parse(
            s.data(), end, ec);
        BOOST_TEST(! ec);
        if(! ec)
        {
            BOOST_TEST(p.value() == v);
            BOOST_TEST(it == end);
        }
    }

    void
    bad_hex(string_view s)
    {
        hex_number p;
        error_code ec;
        auto const end =
            s.data() + s.size();
        p.parse(s.data(), end, ec);
        BOOST_TEST(ec.failed());
    }

    void
    run()
    {
        good_dec("0", 0);
        good_dec("1", 1);
        good_dec("15", 15);
        good_dec(
            "18446744073709551615",
            18446744073709551615ULL);
        good_dec("99999", 99999);
        bad_dec("");
        bad_dec("-1");
        bad_dec("x");
        bad_dec(" 0");
        bad_dec("a");
        bad_dec("18446744073709551616");

        good_hex("0", 0);
        good_hex("9", 9);
        good_hex("a", 10);
        good_hex("A", 10);
        good_hex("F", 15);
        good_hex("FF", 255);
        good_hex("100", 256);
        good_hex(
            "FFFFFFFFFFFFFFFF",
            18446744073709551615ULL);
        bad_hex("");
        bad_hex("-1");
        bad_hex("x");
        bad_hex(" 0");
        bad_hex("g");
        bad_hex("10000000000000000");
        bad_hex("18446744073709551615");
    }
};

TEST_SUITE(number_test, "boost.http_proto.number");

} // bnf
} // http_proto
} // boost
