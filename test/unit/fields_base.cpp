//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_base.hpp>

#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>

#include "test_helpers.hpp"

#include <utility>

namespace boost {
namespace http_proto {

struct fields_base_test
{
#if 0
    void
    modify(
        string_view before,
        void (*pf)(fields_base&),
        string_view after)
    {
        // fields
        {
            fields f0 =
                make_fields(before);
            fields f1 =
                make_fields(after);
            fields f(f0);
            (*pf)(f);
            BOOST_TEST_EQ(f.buffer(),
                f1.buffer());
            test_fields(f, after);
        }

        // request
        {
            request f0 =
                make_request(before);
            request f1 =
                make_request(after);
            request f(f0);
            (*pf)(f);
            BOOST_TEST_EQ(f.buffer(),
                f1.buffer());
            test_fields(f, after);
        }

        // response
        {
            response f0 =
                make_response(before);
            response f1 =
                make_response(after);
            response f(f0);
            (*pf)(f);
            BOOST_TEST_EQ(f.buffer(),
                f1.buffer());
            test_fields(f, after);
        }
    }

    void
    testModifiers()
    {
        string_view const cs =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        // clear()
        {
            {
                fields f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0U);
                f.clear();
                test_fields(f, "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0U);
            }
            {
                fields f = make_fields(cs);
                BOOST_TEST_GT(
                    f.capacity_in_bytes(), 0U);
                f.clear();
                test_fields(f, "\r\n");
                BOOST_TEST_GT(
                    f.capacity_in_bytes(), 0U);
            }
        }

        // reserve(std::size_t)
        {
            {
                fields f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0U);
                f.reserve(100);
                BOOST_TEST_GE(
                    f.capacity_in_bytes(), 100U);
                test_fields(f, "\r\n");
            }
            {
                fields f = make_fields(cs);
                auto const cap =
                    f.capacity_in_bytes();
                BOOST_TEST_GT(cap, 0U);
                f.reserve(cap / 2);
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), cap);
                f.reserve(2 * cap);
                BOOST_TEST_GE(
                    f.capacity_in_bytes(), 2 * cap);
                test_fields(f, cs);
            }
        }

        // shrink_to_fit()
        {
            fields f;
            f.reserve(200);
            f.shrink_to_fit();
            auto const n =
                f.capacity_in_bytes();
            BOOST_TEST_LT(n, 200);
            f.shrink_to_fit();
            BOOST_TEST_EQ(
                f.capacity_in_bytes(), n);
        }

        // swap()
        {
        }
    }

    void
    testAppend()
    {
        // append(field, string_view)

        modify(
            "\r\n",
            [](fields_base& f)
            {
                f.append(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "Cookie: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.append(field::server, "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");

        // append(string_view, string_view)

        modify(
            "\r\n",
            [](fields_base& f)
            {
                f.append("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "Cookie: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.append("Server", "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");
    }

    void
    testInsert()
    {
        // insert(iterator, field, string_view)

        modify(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.insert(f.find("T"),
                    field::server, "x");
            },
            "Server: x\r\n"
            "T: 1\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.insert(f.find("U"),
                    field::server, "x");
            },
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "\r\n");

        // insert(iterator, string_view, string_view)

        modify(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.insert(f.find("T"),
                    "Server", "x");
            },
            "Server: x\r\n"
            "T: 1\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.insert(f.find("U"),
                    "Server", "x");
            },
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "\r\n");

        // self-intersect
        modify(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.insert(
                    f.begin(),
                    f.find(field::user_agent)->value,
                    f.find(field::connection)->value);
            },
            "boost: close\r\n"
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n");

        // self-intersect
        modify(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                // reserve
                f.reserve(f.capacity_in_bytes() * 2);
                f.insert(
                    f.begin(),
                    f.find(field::user_agent)->value,
                    f.find(field::connection)->value);
            },
            "boost: close\r\n"
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n");
    }

    void
    testErase()
    {
        // erase(iterator)
        {
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    f.erase(f.find("Server"));
                },
                "\r\n");

            modify(
                "Cookie: x\r\n"
                "Server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    f.erase(f.find("Server"));
                },
                "Cookie: x\r\n"
                "\r\n");
        }

        // erase(field)
        {
            // no match
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::connection), 0U);
                },
                "Server: y\r\n"
                "\r\n");

            // one match
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 1);
                },
                "\r\n");

            // different capitalization
            modify(
                "server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 1);
                },
                "\r\n");

            // three matches, different capitalization
            modify(
                "Server: x\r\n"
                "server: y\r\n"
                "SERVER: z\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 3);
                },
                "\r\n");

            // three matches, three unmatched
            modify(
                "T: 1\r\n"
                "Server: x\r\n"
                "U: 2\r\n"
                "Server: y\r\n"
                "Server: z\r\n"
                "V: 3\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 3);
                },
                "T: 1\r\n"
                "U: 2\r\n"
                "V: 3\r\n"
                "\r\n");
        }

        // erase(string_view)
        {
            // one match, different case
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("server"), 1);
                },
                "\r\n");

            // one match, different case
            modify(
                "server: y\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("Server"), 1);
                },
                "\r\n");

            // three matches
            modify(
                "T: 1\r\n"
                "Server: x\r\n"
                "U: 2\r\n"
                "Server: y\r\n"
                "Server: z\r\n"
                "V: 3\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("Server"), 3);
                },
                "T: 1\r\n"
                "U: 2\r\n"
                "V: 3\r\n"
                "\r\n");

            // three matches, unknown id
            modify(
                "T: 1\r\n"
                "Server: Boost\r\n"
                "T: 2\r\n"
                "Connection: close\r\n"
                "T: 3\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("T"), 3);
                },
                "Server: Boost\r\n"
                "Connection: close\r\n"
                "\r\n");

            // no matches
            modify(
                "Connection: keep-alive\r\n"
                "Server: Boost\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("Accept"), 0U);
                },
                "Connection: keep-alive\r\n"
                "Server: Boost\r\n"
                "\r\n");

            // unknown field name
            modify(
                "Connection: keep-alive\r\n"
                "X: 1\r\n"
                "Server: Boost\r\n"
                "X: 2\r\n"
                "\r\n",
                [](fields_base& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("X"), 2);
                },
                "Connection: keep-alive\r\n"
                "Server: Boost\r\n"
                "\r\n");
        }
    }

    void
    testSet()
    {
        // set(iterator, string_view)

        modify(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "2");
            },
            "T: 2\r\n"
            "\r\n");

        // set(field, string_view)

        modify(
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "x");
            },
            "Server: x\r\n"
            "\r\n");

        modify(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "Server: x\r\n"
            "T: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "T: 1\r\n"
            "T: 2\r\n"
            "Server: y\r\n"
            "\r\n");

        modify(
            "Server: x1\r\n"
            "Server: x2\r\n"
            "Server: x3\r\n"
            "T: t\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "T: t\r\n"
            "Server: y\r\n"
            "\r\n");

        // set(string_view, string_view)

        modify(
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "x");
            },
            "Server: x\r\n"
            "\r\n");

        modify(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "Server: xx\r\n"
            "T: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "T: 1\r\n"
            "T: 2\r\n"
            "Server: y\r\n"
            "\r\n");

        modify(
            "Server: x1\r\n"
            "Server: x2\r\n"
            "Server: x3\r\n"
            "T: t\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "T: t\r\n"
            "Server: y\r\n"
            "\r\n");

        modify(
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("Connection"), "close");
            },
            "Connection: close\r\n"
            "Server: Boost\r\n"
            "\r\n");
    }
#endif

    void
    testExpect()
    {
        // parse request
        {
            auto const check =
            []( metadata::expect_t md,
                string_view s)
            {
                auto const req =
                    make_request(s);
                BOOST_TEST_EQ(
                    req.metadata().expect.ec,
                    md.ec);
                BOOST_TEST_EQ(
                    req.metadata().expect.count,
                    md.count);
                BOOST_TEST_EQ(
                    req.metadata().expect.is_100_continue,
                    md.is_100_continue);
            };

            check(
                { {}, 0, false},
                "POST / HTTP/1.1\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { error::bad_expect, 1, false },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                { error::bad_expect, 2, false },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { error::bad_expect, 2, false },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");
        }

        // parse response
        {
            auto const check =
            [](string_view s)
            {
                auto const res =
                    make_response(s);
                BOOST_TEST_EQ(
                    res.metadata().expect.ec,
                    error_code());
                BOOST_TEST_EQ(
                    res.metadata().expect.count,
                    res.count(field::expect));
                BOOST_TEST_EQ(
                    res.metadata().expect.is_100_continue,
                    false);
            };

            check(
                "HTTP/1.1 200 OK\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");
        }

        // erase in response
        {
            auto res = make_response(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");
            auto it = res.find(field::expect);
            res.erase(it);
            BOOST_TEST(
                ! res.metadata().expect.ec.failed());
            BOOST_TEST_EQ(
                res.metadata().expect.count, 0);
            BOOST_TEST_EQ(
                res.metadata().expect.is_100_continue,
                false);
        }

        // erase, set in request
        {
            auto const check =
            []( metadata::expect_t md,
                void(*fn)(request&),
                string_view s)
            {
                auto req =
                    make_request(s);
                fn(req);
                BOOST_TEST_EQ(
                    req.metadata().expect.ec,
                    md.ec);
                BOOST_TEST_EQ(
                    req.metadata().expect.count,
                    md.count);
                BOOST_TEST_EQ(
                    req.metadata().expect.is_100_continue,
                    md.is_100_continue);
            };

            // erase

            check(
                { {}, 0, false },
                [](request& req)
                {
                    // erase one
                    auto it = req.find(
                        field::expect);
                    req.erase(it);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { {}, 0, false },
                [](request& req)
                {
                    // erase all
                    BOOST_TEST_EQ(req.erase(
                        field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { {}, 0, false },
                [](request& req)
                {
                    BOOST_TEST_EQ(req.erase(
                        field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    auto it = req.find(field::expect);
                    BOOST_TEST_NE(it, req.end());
                    req.erase(it);
                    BOOST_TEST_EQ(
                        req.count(field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { error::bad_expect, 1, false },
                [](request& req)
                {
                    auto it = req.find(field::expect);
                    BOOST_TEST_NE(it, req.end());
                    req.erase(it);
                    BOOST_TEST_EQ(
                        req.count(field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    auto it = req.find_last(
                        req.end(), field::expect);
                    BOOST_TEST_NE(it, req.end());
                    req.erase(it);
                    BOOST_TEST_EQ(
                        req.count(field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");

            // set

            check(
                { error::bad_expect, 1, false },
                [](request& req)
                {
                    req.set(
                        field::expect,
                        "100-continueish");
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    req.set(
                        field::expect,
                        "100-continue");
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    req.set(
                        field::expect,
                        "100-continue");
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 500-server-error\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");
        }
    }

    void
    run()
    {
#if 0
        testModifiers();
        testAppend();
        testInsert();
        testErase();
        testSet();
#endif

        testExpect();
    }
};

TEST_SUITE(
    fields_base_test,
    "boost.http_proto.fields_base");

} // http_proto
} // boost
