//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/headers.hpp>

#include <boost/http_proto/field.hpp>
#include <boost/http_proto/headers_view.hpp>
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class headers_test
{
public:
    void
    testConversion(
        headers_view)
    {
    }

    void run()
    {
        headers h;
        h.append(field::user_agent, "x");
        h.append(field::connection, "close");
        h.append(field::transfer_encoding, "chunked");
        h.append("a", "1" );
        h.append("b", "2" );
        h.append("a", "3" );
        h.append("c", "4" );

        BOOST_TEST(h.str() ==
            "User-Agent: x\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "a: 1\r\n"
            "b: 2\r\n"
            "a: 3\r\n"
            "c: 4\r\n"
            "\r\n");
        BOOST_TEST(h.size() == 7);
        BOOST_TEST(h[0].value == "x");
        BOOST_TEST(
            h.exists(field::connection));
        BOOST_TEST(! h.exists(field::age));
        BOOST_TEST(h.exists("Connection"));
        BOOST_TEST(h.exists("CONNECTION"));
        BOOST_TEST(! h.exists("connector"));
        BOOST_TEST(h.count(
            field::transfer_encoding) == 1);
        BOOST_TEST(
            h.count(field::age) == 0);
        BOOST_TEST(
            h.count("connection") == 1);
        BOOST_TEST(h.count("a") == 2);
        BOOST_TEST_NO_THROW(
            h.at(field::user_agent) == "x");
        BOOST_TEST_NO_THROW(
            h.at("a") == "1");
        BOOST_TEST_THROWS(h.at(field::age),
            std::exception);
        BOOST_TEST_THROWS(h.at("d"),
            std::exception);
        BOOST_TEST(
            h.value_or("a", "x") == "1");
        BOOST_TEST(
            h.value_or("d", "x") == "x");
        BOOST_TEST(h.value_or(
            field::age, "x") == "x");
        BOOST_TEST(h.value_or(
            field::user_agent, {}) == "x");
        BOOST_TEST(h.find(
            field::connection)->id ==
                field::connection);
        BOOST_TEST(
            h.find("a")->value == "1");
        BOOST_TEST(h.matching(
            field::user_agent).make_list() == "x");
        BOOST_TEST(h.matching(
            "b").make_list() == "2");
        BOOST_TEST(h.matching(
            "a").make_list() == "1,3");

        testConversion(h);
    }
};

TEST_SUITE(
    headers_test,
    "boost.http_proto.headers");

} // http_proto
} // boost
