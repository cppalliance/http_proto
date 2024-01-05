//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_base.hpp>

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/static_assert.hpp>

#include "test_helpers.hpp"

#include <utility>

namespace boost {
namespace http_proto {

// check for overflow
//BOOST_STATIC_ASSERT(
    //fields_base::max_capacity_in_bytes() >= max_offset);

struct fields_base_test
{
    static
    void
    check(
        core::string_view s0,
        void(*fn)(fields_base&),
        core::string_view s1)
    {
        // fields
        {
            fields f(s0);
            fn(f);
            BOOST_TEST_EQ(f.buffer(), s1);
            test_fields(f, s1);
        }

        // request
        {
            auto const m = std::string() +
                "GET / HTTP/1.1\r\n" +
                std::string(s0);
            auto const m1 = std::string() +
                "GET / HTTP/1.1\r\n" +
                std::string(s1);
            request req(m);
            fn(req);
            BOOST_TEST_EQ(req.buffer(), m1);
            test_fields(req, s1);
        }

        // response
        {
            auto const m = std::string() +
                "HTTP/1.1 200 OK\r\n" +
                std::string(s0);
            auto const m1 = std::string() +
                "HTTP/1.1 200 OK\r\n" +
                std::string(s1);
            response res(m);
            fn(res);
            BOOST_TEST_EQ(res.buffer(), m1);
            test_fields(res, s1);
        }
    }

    void
    testCapacity()
    {
        // clear()
        {
            // default fields
            {
                fields f;
                f.clear();
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
            }

            // default request
            {
                request req;
                req.clear();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), 0);
            }

            // default response
            {
                response res;
                res.clear();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), 0);
            }

            {
                fields f(
                    "digest: ffce\r\n"
                    "type: 3\r\n"
                    "\r\n");
                auto const n =
                    f.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                f.clear();
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), n);
            }

            {
                request req(
                    "POST / HTTP/1.1\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n");
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                req.clear();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);
            }

            {
                response res(
                    "HTTP/1.1 404 Not Found\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n");
                auto const n =
                    res.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                res.clear();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), n);
            }
        }

        // reserve_bytes(std::size_t)
        {
            // default request
            {
                request req;
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), 0);

                // first allocation
                req.reserve_bytes(100);
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST_GE(n, 100);
                BOOST_TEST_EQ(req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");

                // no reallocation
                req.reserve_bytes(n);
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);

                // no reallocation
                req.reserve_bytes(n / 2);
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);

                // reallocation
                req.reserve_bytes(n + 1);
                BOOST_TEST_GT(
                    req.capacity_in_bytes(), n);
                BOOST_TEST_EQ(req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
            }

            // response
            {
                core::string_view const cs =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                response res(cs);
                auto n = res.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);

                // no reallocation
                res.reserve_bytes(n);
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), n);
                BOOST_TEST_EQ(res.buffer(), cs);
            }
        }

        // shrink_to_fit()
        {
            // default fields
            {
                fields f;
                f.shrink_to_fit();
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
            }

            // default request
            {
                request req;
                req.shrink_to_fit();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), 0);
            }

            // default response
            {
                response res;
                res.shrink_to_fit();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), 0);
            }

            {
                core::string_view const cs = 
                    "digest: ffce\r\n"
                    "type: 3\r\n"
                    "\r\n";
                fields f(cs);
                f.reserve_bytes(
                    f.capacity_in_bytes() * 2);
                auto const n =
                    f.capacity_in_bytes();
                f.shrink_to_fit();
                BOOST_TEST_LT(
                    f.capacity_in_bytes(), n);
                BOOST_TEST_EQ(f.buffer(), cs);
            }

            {
                core::string_view const cs =
                    "POST / HTTP/1.1\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                request req(cs);
                req.reserve_bytes(
                    req.capacity_in_bytes() * 2);
                auto const n =
                    req.capacity_in_bytes();
                req.shrink_to_fit();
                BOOST_TEST_LT(
                    req.capacity_in_bytes(), n);
                BOOST_TEST_EQ(req.buffer(), cs);
            }

            {
                core::string_view const cs =
                    "HTTP/1.1 404 Not Found\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                response res(cs);
                res.reserve_bytes(
                    res.capacity_in_bytes() * 2);
                auto const n =
                    res.capacity_in_bytes();
                res.shrink_to_fit();
                BOOST_TEST_LT(
                    res.capacity_in_bytes(), n);
                BOOST_TEST_EQ(res.buffer(), cs);
            }
        }
    }

    void
    testAppend()
    {
        // append(field, string_view)

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
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

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
            "Cookie: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.append("Server", "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");

        // empty value should not
        // have a prepended space
        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append("X", "");
            },
            "X:\r\n"
            "\r\n");
    }

    void
    testInsert()
    {
        // insert(iterator, field, string_view)

        check(
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

        check(
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

        check(
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

        check(
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

        check(
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

        check(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                // reserve
                f.reserve_bytes(f.capacity_in_bytes() * 2);
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

        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.erase(f.find("Server"));
            },
            "\r\n");

        check(
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.erase(f.find("Server"));
            },
            "Cookie: x\r\n"
            "\r\n");

        //
        // erase(field)
        //

        // no match
        check(
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
        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::server), 1);
            },
            "\r\n");

        // different capitalization
        check(
            "server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::server), 1);
            },
            "\r\n");

        // three matches, different capitalization
        check(
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
        check(
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

        //
        // erase(string_view)
        //

        // one match, different case
        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("server"), 1);
            },
            "\r\n");

        // one match, different case
        check(
            "server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("Server"), 1);
            },
            "\r\n");

        // three matches
        check(
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
        check(
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
        check(
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
        check(
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

    void
    testSet()
    {
        // set(iterator, string_view)

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "2");
            },
            "T: 2\r\n"
            "\r\n");

        check(
            "T: abc\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "2");
            },
            "T: 2\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "abcdefghijklmnopqrstuvwxyz");
            },
            "T: abcdefghijklmnopqrstuvwxyz\r\n"
            "\r\n");

        // set(field, string_view)

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "x");
            },
            "Server: x\r\n"
            "\r\n");

        check(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
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

        check(
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

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "x");
            },
            "Server: x\r\n"
            "\r\n");

        check(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
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

        check(
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

        check(
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

    void
    testExpect()
    {
        // parse request
        {
            auto const check =
            []( metadata::expect_t md,
                core::string_view s)
            {
                request const req(s);
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
            [](core::string_view s)
            {
                response const res(s);
                BOOST_TEST_EQ(
                    res.metadata().expect.ec,
                    system::error_code());
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
            response res(
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
                core::string_view s)
            {
                request req(s);
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
        testCapacity();
        testAppend();
        testInsert();
        testErase();
        testSet();
        testExpect();

        test_suite::log <<
            "sizeof(detail::header) == " <<
            sizeof(detail::header) << "\n";
    }
};

TEST_SUITE(
    fields_base_test,
    "boost.http_proto.fields_base");

} // http_proto
} // boost
