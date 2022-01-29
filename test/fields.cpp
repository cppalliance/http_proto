//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields.hpp>

#include <boost/http_proto/field.hpp>

#include "test_helpers.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct fields_test
{
    void
    testSpecial()
    {
        string_view const cs =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        string_view const cs2 =
            "x: 1\r\n"
            "y: 2\r\n"
            "z: 3\r\n"
            "\r\n";

        // ~fields()
        // fields()
        {
            fields f;
            BOOST_TEST(
                f.buffer() == "\r\n");
            BOOST_TEST(
                f.buffer().data() ==
                fields().buffer().data());
        }

        // fields(fields&&)
        {
            {
                fields f1 = make_fields(cs);
                fields f2(std::move(f1));
                check(f1, 0, "\r\n");
                check(f2, 4, cs);
            }
            {
                fields f1;
                fields f2(std::move(f1));
                check(f1, 0, "\r\n");
                check(f2, 0, "\r\n");
                BOOST_TEST(
                    f1.buffer().data() ==
                    f2.buffer().data());
            }
        }

        // fields(fields)
        {
            {
                fields f1 = make_fields(cs);
                fields f2(f1);
                check(f1, 4, cs);
                check(f2, 4, cs);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
            {
                fields f1;
                fields f2(f1);
                check(f1, 0, "\r\n");
                check(f2, 0, "\r\n");
                BOOST_TEST(
                    f1.buffer().data() ==
                    f2.buffer().data());
            }
        }

        // fields(fields_view)
        {
            fields_view fv = make_fields(cs);
            BOOST_TEST(
                fv.buffer().data() == cs.data());

            fields f(fv);
            BOOST_TEST(f.buffer() == cs);
            BOOST_TEST(
                f.buffer().data() != cs.data());
            check(f, 4, cs);
        }

        // operator=(fields&&)
        {
            {
                fields f1 = make_fields(cs);
                fields f2;
                f2 = std::move(f1);
                check(f1, 0, "\r\n");
                check(f2, 4, cs);
            }
            {
                fields f1 = make_fields(cs);
                fields f2 = make_fields(cs2);
                f2 = std::move(f1);
                check(f1, 0, "\r\n");
                check(f2, 4, cs);
            }
            {
                fields f1;
                fields f2 = make_fields(cs);
                f2 = std::move(f1);
                check(f1, 0, "\r\n");
                check(f2, 0, "\r\n");
                BOOST_TEST(
                    f1.buffer().data() ==
                    f2.buffer().data());
            }
        }

        // operator=(fields)
        {
            {
                fields f1 = make_fields(cs);
                fields f2;
                f2 = f1;
                check(f1, 4, cs);
                check(f2, 4, cs);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
            {
                fields f1 = make_fields(cs);
                fields f2 = make_fields(
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "z: 3\r\n"
                    "\r\n");
                f2 = f1;
                check(f1, 4, cs);
                check(f2, 4, cs);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
            {
                fields f1;
                fields f2 = make_fields(cs);
                f2 = f1;
                check(f1, 0, "\r\n");
                check(f2, 0, "\r\n");
                BOOST_TEST(
                    f1.buffer().data() ==
                    f2.buffer().data());
            }
        }

        // operator=(fields_view)
        {
            {
                fields_view f1 =
                    make_fields(cs);
                fields f2;
                f2 = f1;
                check(f1, 4, cs);
                check(f2, 4, cs);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
            {
                fields_view f1 =
                    make_fields(cs);
                fields f2 = make_fields(
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "z: 3\r\n"
                    "\r\n");
                f2 = f1;
                check(f1, 4, cs);
                check(f2, 4, cs);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
            {
                fields_view f1;
                fields f2 = make_fields(cs);
                f2 = f1;
                check(f1, 0, "\r\n");
                check(f2, 0, "\r\n");
                BOOST_TEST(
                    f1.buffer().data() ==
                    f2.buffer().data());
            }

            // existing capacity
            {
                fields f1 = make_fields(cs);
                fields f2;
                f2.reserve(2 * cs.size());
                f2 = f1;
                check(f2, 4, cs);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
        }
    }

    void
    testObservers()
    {
        // capacity_in_bytes()
        {
            {
                fields f;
                BOOST_TEST(
                    f.capacity_in_bytes() == 0);
            }
            {
                fields f;
                f.reserve(100);
                BOOST_TEST(
                    f.capacity_in_bytes() >= 100);
            }
            {
                fields f;
                f.reserve(100);
                f.shrink_to_fit();
                BOOST_TEST(
                    f.capacity_in_bytes() > 0);
            }
        }
    }

    void
    testModifiers()
    {
        // clear()
        {
            fields_view v =
                make_fields(
                "Connection: close\r\n"
                "Set-Cookie: 0\r\n"
                "User-Agent: boost\r\n"
                "Set-Cookie: 1\r\n"
                "\r\n");
            {
                fields f;
                BOOST_TEST(f.capacity_in_bytes() == 0);
                f.clear();
                check(f, 0, "\r\n");
                BOOST_TEST(f.capacity_in_bytes() == 0);
            }
            {
                fields f(v);
                BOOST_TEST(f.capacity_in_bytes() > 0);
                f.clear();
                check(f, 0, "\r\n");
                BOOST_TEST(f.capacity_in_bytes() > 0);
            }
        }

        // reserve(std::size_t)
        {
            {
                fields f;
                BOOST_TEST(f.capacity_in_bytes() == 0);
                f.reserve(100);
                BOOST_TEST(f.capacity_in_bytes() >= 100);
                check(f, 0, "\r\n");
            }
            {
                fields_view v =
                    make_fields(
                    "Connection: close\r\n"
                    "Set-Cookie: 0\r\n"
                    "User-Agent: boost\r\n"
                    "Set-Cookie: 1\r\n"
                    "\r\n");
                fields f(v);
                auto const cap =
                    f.capacity_in_bytes();
                BOOST_TEST(cap > 0);
                f.reserve(cap / 2);
                BOOST_TEST(f.capacity_in_bytes() == cap);
                f.reserve(2 * cap);
                BOOST_TEST(f.capacity_in_bytes() >= 2 * cap);
                check(f, 4,
                    "Connection: close\r\n"
                    "Set-Cookie: 0\r\n"
                    "User-Agent: boost\r\n"
                    "Set-Cookie: 1\r\n"
                    "\r\n");
            }
        }

        // shrink_to_fit()
        {
            fields f;
            f.reserve(200);
            f.shrink_to_fit();
            auto const n =
                f.capacity_in_bytes();
            BOOST_TEST(n < 200);
            f.shrink_to_fit();
            BOOST_TEST(
                f.capacity_in_bytes() == n);
        }

        {
            fields_view v =
                make_fields(
                "Connection: close\r\n"
                "Set-Cookie: 0\r\n"
                "User-Agent: boost\r\n"
                "Set-Cookie: 1\r\n"
                "\r\n");

            // emplace(iterator, field, string_view)
            {
                fields f(v);
                f.emplace(
                    f.find(field::user_agent),
                    field::content_length,
                    "42");
                check(f, 5,
                    "Connection: close\r\n"
                    "Set-Cookie: 0\r\n"
                    "Content-Length: 42\r\n"
                    "User-Agent: boost\r\n"
                    "Set-Cookie: 1\r\n"
                    "\r\n");
            }

            // emplace(iterator, string_view, string_view)
            {
                fields f(v);
                f.emplace(
                    f.find(field::set_cookie),
                    "Content-Length",
                    "42");
                check(f, 5,
                    "Connection: close\r\n"
                    "Content-Length: 42\r\n"
                    "Set-Cookie: 0\r\n"
                    "User-Agent: boost\r\n"
                    "Set-Cookie: 1\r\n"
                    "\r\n");
            }

            // emplace(iterator, string_view, string_view)
            {
                fields f(v);
                f.reserve(f.capacity_in_bytes() * 2);
                f.emplace(
                    f.find(field::set_cookie),
                    "Content-Length",
                    "42");
                check(f, 5,
                    "Connection: close\r\n"
                    "Content-Length: 42\r\n"
                    "Set-Cookie: 0\r\n"
                    "User-Agent: boost\r\n"
                    "Set-Cookie: 1\r\n"
                    "\r\n");
            }
        }

        // emplace_back(field, string_view)
        // emplace_back(string_view, string_view)
        {
            fields f;

            // empty
            f.emplace_back("x", "y");
            check(f, 1,
                "x: y\r\n"
                "\r\n");

            // grow
            f.emplace_back(
                field::user_agent, "boost");
            check(f, 2,
                "x: y\r\n"
                "User-Agent: boost\r\n"
                "\r\n");

            f.reserve(200);
            f.emplace_back(
                "Connection", "close");
            check(f, 3,
                "x: y\r\n"
                "User-Agent: boost\r\n"
                "Connection: close\r\n"
                "\r\n");
        }

        // erase(iterator)
        {
            fields_view v =
                make_fields(
                "Connection: close\r\n"
                "Content-Length: 0\r\n"
                "x: 1\r\n"
                "User-Agent: boost\r\n"
                "\r\n");

            {
                fields f(v);
                auto it = f.begin();
                it = f.erase(it);
                BOOST_TEST(it->id ==
                    field::content_length);
                check(f, 3,
                    "Content-Length: 0\r\n"
                    "x: 1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }

            {
                fields f(v);
                auto it = f.begin();
                ++it;
                it = f.erase(it);
                BOOST_TEST(it->name == "x");
                check(f, 3,
                    "Connection: close\r\n"
                    "x: 1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }

            {
                fields f(v);
                auto it = f.begin();
                ++it;
                ++it;
                it = f.erase(it);
                BOOST_TEST(it->id ==
                    field::user_agent);
                check(f, 3,
                    "Connection: close\r\n"
                    "Content-Length: 0\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        {
            fields_view v =
                make_fields(
                "Connection: close\r\n"
                "x: 1\r\n"
                "Set-Cookie: 0\r\n"
                "y: 2\r\n"
                "x: 3\r\n"
                "z: 4\r\n"
                "x: 5\r\n"
                "Set-Cookie: 1\r\n"
                "y: 5\r\n"
                "\r\n");

            // erase(field)
            {
                fields f(v);

                BOOST_TEST(
                    f.erase(field::range) == 0);

                BOOST_TEST(
                    f.erase(field::set_cookie) == 1);
                check(f, 8,
                    "Connection: close\r\n"
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "x: 3\r\n"
                    "z: 4\r\n"
                    "x: 5\r\n"
                    "Set-Cookie: 1\r\n"
                    "y: 5\r\n"
                    "\r\n");
            }

            // erase(string_view)
            {
                fields f(v);

                BOOST_TEST(f.erase("q") == 0);

                BOOST_TEST(
                    f.erase("Set-Cookie") == 1);
                check(f, 8,
                    "Connection: close\r\n"
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "x: 3\r\n"
                    "z: 4\r\n"
                    "x: 5\r\n"
                    "Set-Cookie: 1\r\n"
                    "y: 5\r\n"
                    "\r\n");

                f.erase("x");
                check(f, 7,
                    "Connection: close\r\n"
                    "y: 2\r\n"
                    "x: 3\r\n"
                    "z: 4\r\n"
                    "x: 5\r\n"
                    "Set-Cookie: 1\r\n"
                    "y: 5\r\n"
                    "\r\n");
            }

            // erase_all(field)
            {
                fields f(v);

                BOOST_TEST(
                    f.erase_all(field::range) == 0);

                BOOST_TEST(
                    f.erase_all(field::set_cookie) == 2);
                check(f, 7,
                    "Connection: close\r\n"
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "x: 3\r\n"
                    "z: 4\r\n"
                    "x: 5\r\n"
                    "y: 5\r\n"
                    "\r\n");

                BOOST_TEST(
                    f.erase_all(field::connection) == 1);
                check(f, 6,
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "x: 3\r\n"
                    "z: 4\r\n"
                    "x: 5\r\n"
                    "y: 5\r\n"
                    "\r\n");
            }

            // erase_all(string_view)
            {
                fields f(v);

                BOOST_TEST(
                    f.erase_all("q") == 0);

                // lowercase
                BOOST_TEST(
                    f.erase_all("set-cookie") == 2);
                check(f, 7,
                    "Connection: close\r\n"
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "x: 3\r\n"
                    "z: 4\r\n"
                    "x: 5\r\n"
                    "y: 5\r\n"
                    "\r\n");

                // lowercase
                BOOST_TEST(
                    f.erase_all("X") == 3);
                check(f, 4,
                    "Connection: close\r\n"
                    "y: 2\r\n"
                    "z: 4\r\n"
                    "y: 5\r\n"
                    "\r\n");
            }
        }

        // swap()
        {
        }
    }

    void
    run()
    {
        testSpecial();
        testObservers();
        testModifiers();
    }
};

TEST_SUITE(
    fields_test,
    "boost.http_proto.fields");

} // http_proto
} // boost
