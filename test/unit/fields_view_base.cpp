//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/static_assert.hpp>
#include <type_traits>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

struct fields_view_base_test
{
    void
    testIterators()
    {
        fields_view_base const& f =
            make_fields(
                "x: 1\r\n"
                "Accept: 2\r\n"
                "z: 3\r\n"
                "\r\n");

        // iterator
        // begin()
        // end()
        {
            BOOST_STATIC_ASSERT(std::is_same<
                fields_view_base::iterator,
                fields_view_base::const_iterator>::value);

            BOOST_TEST(
                fields_view_base::iterator() ==
                fields_view_base::iterator());

            fields_view_base::iterator it = f.begin();
            BOOST_TEST(it == f.begin());
            BOOST_TEST(it != f.end());

            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "x");
            BOOST_TEST_EQ(it->value, "1");

            ++it;
            BOOST_TEST_EQ(it->id, field::accept);
            BOOST_TEST_EQ(it->name, "Accept");
            BOOST_TEST_EQ(it->value, "2");

            {
                auto it0 = it++; // post-increment
                BOOST_TEST_EQ(it0->id, field::accept);
                BOOST_TEST_EQ(it0->name, "Accept");
                BOOST_TEST_EQ(it0->value, "2");
            }
            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "z");
            BOOST_TEST_EQ(it->value, "3");

            ++it;
            BOOST_TEST_EQ(it, f.end());

            --it;
            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "z");
            BOOST_TEST_EQ(it->value, "3");

            {
                auto it1 = it--; // post-decrement
                BOOST_TEST_EQ(it1->id, field::unknown);
                BOOST_TEST_EQ(it1->name, "z");
                BOOST_TEST_EQ(it1->value, "3");
            }
            BOOST_TEST_EQ(it->id, field::accept);
            BOOST_TEST_EQ(it->name, "Accept");
            BOOST_TEST_EQ(it->value, "2");

            --it;
            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "x");
            BOOST_TEST_EQ(it->value, "1");

            BOOST_TEST_EQ(it, f.begin());
        }

        // reverse_iterator
        // rbegin()
        // rend()
        {
            BOOST_STATIC_ASSERT(std::is_same<
                fields_view_base::reverse_iterator,
                fields_view_base::const_reverse_iterator>::value);

            BOOST_TEST(
                fields_view_base::reverse_iterator() ==
                fields_view_base::reverse_iterator());

            fields_view_base::reverse_iterator it = f.rbegin();
            BOOST_TEST(it == f.rbegin());
            BOOST_TEST(it != f.rend());

            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "z");
            BOOST_TEST_EQ(it->value, "3");

            ++it;
            BOOST_TEST_EQ(it->id, field::accept);
            BOOST_TEST_EQ(it->name, "Accept");
            BOOST_TEST_EQ(it->value, "2");

            {
                auto it0 = it++; // post-increment
                BOOST_TEST_EQ(it0->id, field::accept);
                BOOST_TEST_EQ(it0->name, "Accept");
                BOOST_TEST_EQ(it0->value, "2");
            }
            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "x");
            BOOST_TEST_EQ(it->value, "1");

            ++it;
            BOOST_TEST_EQ(it, f.rend());

            --it;
            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "x");
            BOOST_TEST_EQ(it->value, "1");

            {
                auto it0 = it--; // post-decrement
                BOOST_TEST_EQ(it0->id, field::unknown);
                BOOST_TEST_EQ(it0->name, "x");
                BOOST_TEST_EQ(it0->value, "1");
            }
            BOOST_TEST_EQ(it->id, field::accept);
            BOOST_TEST_EQ(it->name, "Accept");
            BOOST_TEST_EQ(it->value, "2");

            --it;
            BOOST_TEST_EQ(it->id, field::unknown);
            BOOST_TEST_EQ(it->name, "z");
            BOOST_TEST_EQ(it->value, "3");

            BOOST_TEST_EQ(it, f.rbegin());
        }
    }

    void
    testObservers()
    {
        fields_view_base const& f =
            make_fields(
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
                "\r\n");

        // size()

        BOOST_TEST(f.size() == 10);

        // exists(field)
        // exists(string_view)

        BOOST_TEST(f.exists("x"));
        BOOST_TEST(f.exists("X"));
        BOOST_TEST(f.exists("user-agent"));
        BOOST_TEST(! f.exists("a"));
        BOOST_TEST(f.exists(field::user_agent));
        BOOST_TEST(! f.exists(field::range));

        // count(field)
        // count(string_view)

        BOOST_TEST(f.count("x") == 3);
        BOOST_TEST(f.count("y") == 1);
        BOOST_TEST(f.count("q") == 0);
        BOOST_TEST(f.count(field::range) == 0);
        BOOST_TEST(f.count(field::user_agent) == 1);

        // find(field)
        // find(string_view)
        {
            BOOST_TEST(f.find("x")->name == "x");
            BOOST_TEST(f.find("accept") == f.end());
            BOOST_TEST(f.find(field::range) == f.end());
            BOOST_TEST(f.find(field::user_agent)->value == "boost");
        }

        // find(iterator, field)
        // find(iterator, string_view)
        {
            auto const it = f.find("y");
            BOOST_TEST(it != f.end());
            BOOST_TEST(it->value == "2");

            BOOST_TEST(f.find(it, "x")->value == "3");
            BOOST_TEST(f.find(it, "q") == f.end());
            BOOST_TEST(f.find(it, field::set_cookie)->value == "a");
            BOOST_TEST(f.find(it, field::range) == f.end());
        }

        // find_last(iterator, field)
        // find_last(iterator, string_view)
        {
            auto const it = f.find_last(f.end(), "x");
            BOOST_TEST(it != f.end());
            BOOST_TEST(it->value == "5");

            BOOST_TEST(f.find_last(it, "x")->value == "3");
            BOOST_TEST(f.find_last(it, "q") == f.end());
            BOOST_TEST(f.find_last(it, field::set_cookie)->value == "b");
            BOOST_TEST(f.find_last(it, field::range) == f.end());
        }

        // value_or(field, string_view)
        // value_or(string_view, string_view)
        {
            BOOST_TEST(f.value_or(
                field::set_cookie, "") == "a");
            BOOST_TEST(f.value_or(
                field::set_cookie2, "Q") == "Q");

            BOOST_TEST(f.value_or(
                "set-cookie", "") == "a");
            BOOST_TEST(f.value_or(
                "set-cookie2", "Q") == "Q");
        }
    }

    void
    testSubrange()
    {
    }

    void
    run()
    {
        testIterators();
        testObservers();
        testSubrange();
    }
};

TEST_SUITE(
    fields_view_base_test,
    "boost.http_proto.fields_view_base");

} // http_proto
} // boost
