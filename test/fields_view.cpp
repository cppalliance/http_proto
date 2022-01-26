//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_view.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct fields_view_test
{
    fields_view
    construct(
        string_view s)
    {
        return fields_view(s);
    }

    void
    testIterator()
    {
        auto f = construct(
            "Content-Length: 42\r\n"
            "User-Agent: boost\r\n"
            "\r\n");

        auto it = f.begin();
        BOOST_TEST(it == f.begin());
        BOOST_TEST(it == f.begin()++);
        BOOST_TEST(it != f.end());
        BOOST_TEST(it !=
            fields_view::iterator());
        BOOST_TEST(it->id ==
            field::content_length);
        BOOST_TEST(it->name ==
            "Content-Length");
        BOOST_TEST(it->value == "42");

        ++it;
        BOOST_TEST(it != f.begin());
        BOOST_TEST(it == ++f.begin());
        BOOST_TEST(it != f.end());
        BOOST_TEST(it !=
            fields_view::iterator());
        BOOST_TEST(it->id ==
            field::user_agent);
        BOOST_TEST(it->name ==
            "User-Agent");
        BOOST_TEST(it->value == "boost");

        ++it;
        BOOST_TEST(it != f.begin());
        BOOST_TEST(it == f.end());
        BOOST_TEST(it !=
            fields_view::iterator());
    }

    void
    testObservers()
    {
        auto f = construct(
            "x: 1\r\n"
            "y: 2\r\n"
            "x: 3\r\n"
            "z: 4\r\n"
            "x: 5\r\n"
            "p: 6\r\n"
            "User-Agent: 7\r\n"
            "\r\n");

        // size
        BOOST_TEST(f.size() == 7);

        // exists
        BOOST_TEST(! f.exists("a"));
        BOOST_TEST(f.exists("x"));
        BOOST_TEST(f.exists("X"));
        BOOST_TEST(f.exists("user-agent"));
        BOOST_TEST(! f.exists(
            field::content_length));
        BOOST_TEST(f.exists(
            field::user_agent));

        // count
        BOOST_TEST(f.count("x") == 3);
        BOOST_TEST(f.count("y") == 1);
        BOOST_TEST(f.count("q") == 0);
        BOOST_TEST(f.count(
            field::content_length) == 0);
        BOOST_TEST(f.count(
            field::user_agent) == 1);

        // at
        BOOST_TEST(f.at("x") == "1");
        BOOST_TEST(f.at(
            field::user_agent) == "7");
        BOOST_TEST_THROWS(f.at("q"),
            std::invalid_argument);
        BOOST_TEST_THROWS(f.at(
            field::content_length),
            std::invalid_argument);

        // value_or
        BOOST_TEST(f.value_or(
            field::content_length, "42") == "42");
        BOOST_TEST(f.value_or(
            field::user_agent, "42") == "7");
        BOOST_TEST(f.value_or("x", "*") == "1");
        BOOST_TEST(f.value_or("q", "*") == "*");

        // find
        {
            auto it = f.find("z");
            BOOST_TEST(it->value == "4");
            it = f.find("x");
            BOOST_TEST(it->value == "1");
            it = f.find(field::content_length);
            BOOST_TEST(it == f.end());
            it = f.find(field::user_agent);
            BOOST_TEST(it->id ==
                field::user_agent);
        }

        // find_all
        // make_list
        BOOST_TEST(
            make_list(f.find_all("x")) ==
            "1,3,5");
        BOOST_TEST(
            make_list(f.find_all("y")) ==
            "2");
        BOOST_TEST(
            make_list(f.find_all("q")) == "");
        BOOST_TEST(
            make_list(f.find_all(
                field::user_agent)) == "7");
    }

    void
    testSubrange()
    {
        using S =
            fields_view::subrange;

        auto f = construct(
            "x: 1\r\n"
            "y: 2\r\n"
            "x: 3\r\n"
            "z: 4\r\n"
            "x: 5\r\n"
            "p: 6\r\n"
            "User-Agent: 7\r\n"
            "\r\n");

        // subrange
        {
            S sr;
            BOOST_TEST(
                sr.begin() == sr.end());
        }

        // subrange(subrange const&)
        {
            S sr;
            S sr2(sr);
            BOOST_TEST(
                sr.begin() == sr.end());
        }

        // operator=(subrange const&)
        {
            S sr = f.find_all("x");
            S sr2;
            sr2 = sr;
            BOOST_TEST(
                sr2.begin() == sr.begin());
        }

        // iterators
        S sr = f.find_all("x");
        auto it = sr.begin();
        BOOST_TEST(it == sr.begin());
        BOOST_TEST(it == sr.begin()++);
        BOOST_TEST(it != sr.end());
        BOOST_TEST(it != S::iterator());
        BOOST_TEST(
            it->id == field::unknown);
        BOOST_TEST(it->name == "x");
        BOOST_TEST(it->value == "1");

        ++it;
        BOOST_TEST(it != sr.begin());
        BOOST_TEST(it == ++sr.begin());
        BOOST_TEST(it != sr.end());
        BOOST_TEST(it != S::iterator());
        BOOST_TEST(it->name == "x");
        BOOST_TEST(it->value == "3");

        ++it;
        BOOST_TEST(it != sr.begin());
        BOOST_TEST(it != sr.end());
        BOOST_TEST(it != S::iterator());
        BOOST_TEST(it->name == "x");
        BOOST_TEST(it->value == "5");

        ++it;
        BOOST_TEST(it == sr.end());
    }

    void
    run()
    {
        testIterator();
        testObservers();
        testSubrange();
    }
};

TEST_SUITE(
    fields_view_test,
    "boost.http_proto.fields_view");

} // http_proto
} // boost
