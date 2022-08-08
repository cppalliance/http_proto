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

#include <boost/http_proto/field.hpp>

#include "test_helpers.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct fields_view_test
{
    string_view const cs_ =
        "Content-Length: 42\r\n"
        "x: 1\r\n"
        "y: 2\r\n"
        "Set-Cookie: a\r\n"
        "x: 3\r\n"
        "z: 4\r\n"
        "Set-Cookie: b\r\n"
        "x: 5\r\n"
        "p: 6\r\n"
        "User-Agent: boost\r\n"
        "\r\n";

    void
    testOneView(fields_view f)
    {
        // size
        BOOST_TEST(f.size() == 10);

        // exists
        BOOST_TEST(! f.exists("a"));
        BOOST_TEST(f.exists("x"));
        BOOST_TEST(f.exists("X"));
        BOOST_TEST(f.exists("user-agent"));
        BOOST_TEST(! f.exists(
            field::range));
        BOOST_TEST(f.exists(
            field::user_agent));

        // count
        BOOST_TEST(f.count("x") == 3);
        BOOST_TEST(f.count("y") == 1);
        BOOST_TEST(f.count("q") == 0);
        BOOST_TEST(f.count(
            field::range) == 0);
        BOOST_TEST(f.count(
            field::user_agent) == 1);

        // at
        BOOST_TEST(f.at("x") == "1");
        BOOST_TEST(f.at(
            field::user_agent) == "boost");
        BOOST_TEST_THROWS(f.at("q"),
            std::invalid_argument);
        BOOST_TEST_THROWS(f.at(
            field::range),
            std::invalid_argument);

        // value_or
        BOOST_TEST(f.value_or(
            field::content_length, "69") == "42");
        BOOST_TEST(f.value_or(
            field::user_agent, "42") == "boost");
        BOOST_TEST(f.value_or(
            field::range, "0-1") == "0-1");
        BOOST_TEST(f.value_or("x", "*") == "1");
        BOOST_TEST(f.value_or("q", "*") == "*");

        // find
        {
            auto it = f.find("z");
            BOOST_TEST(it->value == "4");
            it = f.find("x");
            BOOST_TEST(it->value == "1");
            it = f.find(field::range);
            BOOST_TEST(it == f.end());
            it = f.find(field::user_agent);
            BOOST_TEST(it->id ==
                field::user_agent);

            it = f.find(++(++f.begin()), "x");
            BOOST_TEST(it->value == "3");

            it = f.find(++f.begin(),
                field::user_agent);
            BOOST_TEST(it->value == "boost");
        }

        // find_all
        // make_list
        std::string s;
        BOOST_TEST(
            make_list(f.find_all("x"), s) ==
            "1,3,5");
        BOOST_TEST(
            make_list(f.find_all("y"), s) ==
            "2");
        BOOST_TEST(
            make_list(f.find_all("q"), s) == "");
        BOOST_TEST(
            make_list(f.find_all(
                field::user_agent), s) == "boost");
        BOOST_TEST(
            make_list(f.find_all(
                field::set_cookie), s) == "a,b");

        // iterator
        BOOST_TEST(
            fields_view::iterator() ==
            fields_view::iterator());

        auto it = f.begin();
        BOOST_TEST(it == f.begin());
        BOOST_TEST(it == f.begin()++);
        BOOST_TEST(it != f.end());
        BOOST_TEST(it->id ==
            field::content_length);
        BOOST_TEST(it->name ==
            "Content-Length");
        BOOST_TEST(it->value == "42");

        ++it;
        ++it;
        ++it;
        BOOST_TEST(it != f.begin());
        BOOST_TEST(it == ++(++(++f.begin())));
        BOOST_TEST(it != f.end());
        BOOST_TEST(it->id ==
            field::set_cookie);
        BOOST_TEST(it->name ==
            "Set-Cookie");
        BOOST_TEST(it->value == "a");

        ++it;
        BOOST_TEST(it != f.begin());
        BOOST_TEST(it != f.end());

        test_fields(f, cs_);
    }

    void
    testViews()
    {
#if 0
        // fields_view()
        {
            fields_view f;
            BOOST_TEST(f.size() == 0);
            BOOST_TEST(f.begin() == f.end());
            BOOST_TEST(f.string() == "\r\n");
        }

        {
            fields_view f =
                make_fields(cs_);
            testOneView(f);
        }

        // with lookup table
        {
            std::string s;
            fields_view f =
                make_fields(cs_, s);
            testOneView(f);
        }

        // swap
        {
            string_view const cs2 =
                "t: 1\r\n"
                "u: 2\r\n"
                "v: 3\r\n"
                "\r\n";
            fields_view f1 = make_fields(cs_);
            fields_view f2 = make_fields(cs2);
            using std::swap;
            swap(f1, f2);
            BOOST_TEST(
                f1.string().data() == cs2.data());
            BOOST_TEST(
                f2.string().data() == cs_.data());
        }
#endif
    }

    void
    testSubrange()
    {
#if 0
        using S =
            fields_view::subrange;

        fields_view f =
            make_fields(
            "x: 1\r\n"
            "y: 2\r\n"
            "x: 3\r\n"
            "z: 4\r\n"
            "x: 5\r\n"
            "p: 6\r\n"
            "User-Agent: boost\r\n"
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
        BOOST_TEST(
            S::iterator() ==
            S::iterator());

        {
            S sr = f.find_all("x");
            auto it = sr.begin();
            BOOST_TEST(it == sr.begin());
            BOOST_TEST(it == sr.begin()++);
            BOOST_TEST(it != sr.end());
            BOOST_TEST(
                it->id == field::unknown);
            BOOST_TEST(it->name == "x");
            BOOST_TEST(it->value == "1");

            ++it;
            BOOST_TEST(it != sr.begin());
            BOOST_TEST(it == ++sr.begin());
            BOOST_TEST(it != sr.end());
            BOOST_TEST(it->name == "x");
            BOOST_TEST(it->value == "3");

            ++it;
            BOOST_TEST(it != sr.begin());
            BOOST_TEST(it != sr.end());
            BOOST_TEST(it->name == "x");
            BOOST_TEST(it->value == "5");

            ++it;
            BOOST_TEST(it == sr.end());
        }

        {
            S sr = f.find_all(
                field::user_agent);
            auto it = sr.begin();
            BOOST_TEST(it->value == "boost");
        }
#endif
    }

    void
    run()
    {
        // fields_view()
        {
            fields_view fv;
        }

        // fields_view(fields_view const&)
        {
            {
                fields_view f1;
                fields_view f2(f1);
            }
        }

        // operator=(fields_view const&)
        {
            {
                fields_view f1;
                fields_view f2;
                f1 = f2;
            }
        }

        testViews();
        testSubrange();
    }
};

TEST_SUITE(
    fields_view_test,
    "boost.http_proto.fields_view");

} // http_proto
} // boost
