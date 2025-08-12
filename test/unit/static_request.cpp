//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/static_request.hpp>

#include <boost/http_proto/request_view.hpp>

#include <utility>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct static_request_test
{
    template<std::size_t Capacity>
    static
    void
    check(
        static_request<Capacity>& req,
        std::size_t count,
        core::string_view s)
    {
        req = static_request<Capacity>(s);
        BOOST_TEST(
            req.size() == count);
    }

    void
    testHelpers()
    {
        core::string_view const cs =
            "POST /x HTTP/1.0\r\n"
            "User-Agent: boost\r\n"
            "\r\n";
        static_request<64> req(cs);
        BOOST_TEST(req.method() == method::post);
        BOOST_TEST(req.method_text() == "POST");
        BOOST_TEST(req.target() == "/x");
        BOOST_TEST(req.version() == version::http_1_0);
        BOOST_TEST(req.buffer().data() != cs.data());
        BOOST_TEST(req.buffer() == cs);
    }

    void
    testSpecial()
    {

        core::string_view const cs =
            "POST /x HTTP/1.0\r\n"
            "Content-Length: 42\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        // static_request()
        {
            static_request<64> req;
            check(req, 0,
                "GET / HTTP/1.1\r\n"
                "\r\n");
            BOOST_TEST(
                req.method() == method::get);
            BOOST_TEST(
                req.method_text() == "GET");
            BOOST_TEST(
                req.target() == "/");
            BOOST_TEST(
                req.version() == version::http_1_1);
        }

        // static_request(static_request const&)
        {
            {
                // default
                static_request<64> r1;
                static_request<64> r2(r1);
                check(r2, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r2.method() == method::get);
                BOOST_TEST(
                    r2.method_text() == "GET");
                BOOST_TEST(
                    r2.version() == version::http_1_1);
            }
            {
                static_request<128> r1(cs);
                static_request<128> r2(r1);
                check(r1, 2, cs);
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.buffer().data() !=
                        r1.buffer().data());
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_text() == "POST");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
        }

        // operator=(static_request const&)
        {
            {
                // default
                static_request<128> r1;
                static_request<128> r2(cs);
                r1 = r2;
                check(r1, 2, cs);
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.buffer().data() !=
                        r1.buffer().data());
                BOOST_TEST(
                    r1.method() == method::post);
                BOOST_TEST(
                    r1.method_text() == "POST");
                BOOST_TEST(
                    r1.version() == version::http_1_0);
            }
            {
                static_request<128> r1(cs);
                static_request<128> r2;
                r1 = r2;
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r1.buffer().data() !=
                        r2.buffer().data());
                BOOST_TEST(
                    r1.method() == method::get);
                BOOST_TEST(
                    r1.method_text() == "GET");
                BOOST_TEST(
                    r1.version() == version::http_1_1);
            }
        }
    }

    void
    testViewConstructor()
    {
        {
            static_request<64> req;
            BOOST_TEST_EQ(
                req.buffer(),
                "GET / HTTP/1.1\r\n\r\n");

            request_view req_view(req);
            static_request<64> req2(req_view);

            BOOST_TEST_EQ(
                req2.buffer(),
                "GET / HTTP/1.1\r\n\r\n");

            BOOST_TEST_NE(
                req2.buffer().data(),
                req.buffer().data());
        }

        {
            static_request<64> req;
            req.set_method("POST");
            BOOST_TEST_EQ(
                req.buffer(),
                "POST / HTTP/1.1\r\n\r\n");

            request_view req_view(req);
            static_request<64> req2(req_view);

            BOOST_TEST_EQ(
                req2.buffer(),
                "POST / HTTP/1.1\r\n\r\n");

            BOOST_TEST_NE(
                req2.buffer().data(),
                req.buffer().data());

            BOOST_TEST_NE(
                req2.buffer().data(),
                req_view.buffer().data());
        }
    }

    void
    testObservers()
    {
    }

    void
    testModifiers()
    {
        // clear()
        {
            {
                static_request<64> req;
                BOOST_TEST(req.capacity_in_bytes() == 64);
                req.clear();
                BOOST_TEST(req.capacity_in_bytes() == 64);
                BOOST_TEST(req.buffer() ==
                    "GET / HTTP/1.1\r\n\r\n");
            }
            {
                static_request<128> req(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST(n > 0);
                req.clear();
                BOOST_TEST(req.size() == 0);
                BOOST_TEST(
                    req.capacity_in_bytes() == n);
                BOOST_TEST(
                    req.method() == method::get);
                BOOST_TEST(
                    req.method_text() == "GET");
                BOOST_TEST(
                    req.target() == "/");
                BOOST_TEST(
                    req.version() == version::http_1_1);
                BOOST_TEST(req.buffer() ==
                    "GET / HTTP/1.1\r\n\r\n");
            }
        }

        // set_method(method)
        {
            {
                static_request<64> req;
                req.set_method(method::delete_);
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE / HTTP/1.1\r\n\r\n");
            }
            {
                static_request<128> req(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method(method::delete_);
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_method(string_view)
        {
            {
                static_request<64> req;
                req.set_method("DELETE");
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE / HTTP/1.1\r\n\r\n");
            }
            {
                static_request<128> req(
                    "POST /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("DELETE");
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                static_request<128> req(
                    "DELETE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("PUT");
                BOOST_TEST(
                    req.method() == method::put);
                BOOST_TEST(
                    req.method_text() == "PUT");
                BOOST_TEST(req.buffer() ==
                    "PUT /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                static_request<128> req(
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("PUT");
                BOOST_TEST(
                    req.method() == method::put);
                BOOST_TEST(
                    req.method_text() == "PUT");
                BOOST_TEST_EQ(req.buffer(),
                    "PUT /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                static_request<128> req(
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("SOMETHINGSUPERLONGHERE");
                BOOST_TEST(
                    req.method() == method::unknown);
                BOOST_TEST(
                    req.method_text() == "SOMETHINGSUPERLONGHERE");
                BOOST_TEST(req.buffer() ==
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_target
        {
            {
                static_request<128> req;
                req.set_target("/index.htm");
                BOOST_TEST(
                    req.target() == "/index.htm");
                BOOST_TEST(req.buffer() ==
                    "GET /index.htm HTTP/1.1\r\n\r\n");
            }
            {
                static_request<128> req(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_target("/index.htm");
                BOOST_TEST(req.buffer() ==
                    "POST /index.htm HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                // shrinks
                static_request<128> req(
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_target("/abc");
                BOOST_TEST_EQ(
                    req.target(), "/abc");
                BOOST_TEST_EQ(req.buffer(),
                    "SOMETHINGSUPERLONGHERE /abc HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                // same size
                static_request<128> req(
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_target("/zyxwvutsrqponmlkjihgfedcba");
                BOOST_TEST_EQ(
                    req.target(), "/zyxwvutsrqponmlkjihgfedcba");
                BOOST_TEST_EQ(req.buffer(),
                    "SOMETHINGSUPERLONGHERE /zyxwvutsrqponmlkjihgfedcba HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                // grows
                static_request<128> req(
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyz HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_target("/abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba");
                BOOST_TEST_EQ(
                    req.target(), "/abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba");
                BOOST_TEST_EQ(req.buffer(),
                    "SOMETHINGSUPERLONGHERE /abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_version
        {
            {
                static_request<128> req;
                req.set_version(version::http_1_0);
                BOOST_TEST(
                    req.version() == version::http_1_0);
                BOOST_TEST(req.buffer() ==
                    "GET / HTTP/1.0\r\n\r\n");
            }
            {
                static_request<128> req(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_version(version::http_1_0);
                BOOST_TEST(
                    req.version() == version::http_1_0);
                BOOST_TEST(req.buffer() ==
                    "POST /x HTTP/1.0\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }
    }

    void
    testExpect()
    {
        {
            static_request<64> req;
            req.set_expect_100_continue(true);
            BOOST_TEST(
                req.metadata().expect.is_100_continue);

            req.set_expect_100_continue(false);
            BOOST_TEST(
                !req.metadata().expect.is_100_continue);
        }


        {
            static_request<64> req;
            req.set_expect_100_continue(false);
            BOOST_TEST(
                !req.metadata().expect.is_100_continue);
        }

        {
            static_request<64> req;
            req.set_expect_100_continue(true);
            BOOST_TEST(
                req.metadata().expect.is_100_continue);

            req.set_expect_100_continue(true);
            BOOST_TEST(
                req.metadata().expect.is_100_continue);
        }

        {
            static_request<64> req;
            BOOST_TEST(
                !req.metadata().expect.ec.failed());

            req.set("Expect", "101-dalmations");
            BOOST_TEST(
                req.metadata().expect.ec.failed());

            req.set_expect_100_continue(true);
            BOOST_TEST(
                !req.metadata().expect.ec.failed());
            BOOST_TEST(
                req.metadata().expect.is_100_continue);
        }

        {
            static_request<128> req;
            req.append("Expect", "100-continue");
            req.append("Expect", "101-dalmations");

            BOOST_TEST(
                req.metadata().expect.ec.failed());
            BOOST_TEST_EQ(
                req.count("Expect"), 2);

            req.set_expect_100_continue(true);
            BOOST_TEST(
                !req.metadata().expect.ec.failed());
            BOOST_TEST(
                req.metadata().expect.is_100_continue);
            BOOST_TEST_EQ(
                req.count("Expect"), 1);
        }

        {
            static_request<256> req;
            req.append("Expect", "100-continue");
            req.append("Content-Length", "1234");
            req.append("Expect", "100-continue");

            BOOST_TEST(
                req.metadata().expect.ec.failed());
            BOOST_TEST_EQ(
                req.count("Expect"), 2);

            req.set_expect_100_continue(true);
            BOOST_TEST(
                !req.metadata().expect.ec.failed());
            BOOST_TEST(
                req.metadata().expect.is_100_continue);
            BOOST_TEST_EQ(
                req.count("Expect"), 1);
        }

        {
            static_request<256> req;
            req.append("Expect", "404-not-found");
            req.append("Content-Length", "1234");
            req.append("Expect", "101-dalmations");

            BOOST_TEST(
                req.metadata().expect.ec.failed());
            BOOST_TEST_EQ(
                req.count("Expect"), 2);

            req.set_expect_100_continue(true);
            BOOST_TEST(
                !req.metadata().expect.ec.failed());
            BOOST_TEST(
                req.metadata().expect.is_100_continue);
            BOOST_TEST_EQ(
                req.count("Expect"), 1);
        }

        {
            static_request<256> req;
            req.append("Expect", "100-continue");
            req.append("Content-Length", "1234");
            req.append("Expect", "100-continue");

            BOOST_TEST(
                req.metadata().expect.ec.failed());
            BOOST_TEST_EQ(
                req.count("Expect"), 2);

            req.set_expect_100_continue(false);
            BOOST_TEST(
                !req.metadata().expect.ec.failed());
            BOOST_TEST(
                !req.metadata().expect.is_100_continue);
            BOOST_TEST_EQ(
                req.count("Expect"), 0);
            BOOST_TEST_EQ(
                req.count("Content-Length"), 1);
        }
    }

    void
    testInitialSize()
    {
        static_request<96> r;
        r.append("T", "*");
        r.append("T", "*");
        r.append("T", "*");

        BOOST_TEST_THROWS(
            r.append("T", "*"),
            std::length_error);

        BOOST_TEST_THROWS(
            r.set_target("/index.html"),
            std::length_error);

        r.set_target("/");
    }

    void
    run()
    {
        testHelpers();
        testSpecial();
        testViewConstructor();
        testObservers();
        testModifiers();
        testExpect();
        testInitialSize();
    }
};

TEST_SUITE(
    static_request_test,
    "boost.http_proto.static_request");

} // http_proto
} // boost
