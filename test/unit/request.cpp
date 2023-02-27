//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request.hpp>

#include "test_helpers.hpp"

#include <utility>

namespace boost {
namespace http_proto {

struct request_test
{
    void
    testHelpers()
    {
        core::string_view const cs =
            "POST /x HTTP/1.0\r\n"
            "User-Agent: boost\r\n"
            "\r\n";
        request req(cs);
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
        auto const check =
        []( request& req,
            std::size_t count,
            core::string_view s)
        {
            req = request(s);
            BOOST_TEST(
                req.size() == count);
        };

        core::string_view const cs =
            "POST /x HTTP/1.0\r\n"
            "Content-Length: 42\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        core::string_view const cs2 =
            "CONNECT 127.0.0.1 HTTP/1.1\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        // request()
        {
            request req;
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

        // request(request const&)
        {
            {
                // default
                request r1;
                request r2(r1);
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
                request r1(cs);
                request r2(r1);
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

        // request(request&&)
        {
            {
                // default
                request r1;
                request r2(std::move(r1));
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
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
                request r1(cs);
                request r2(std::move(r1));
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
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

        // operator=(request const&)
        {
            {
                // default
                request r1;
                request r2(cs);
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
                request r1(cs);
                request r2;
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

        // operator=(fields&&)
        {
            {
                request r1(cs);
                request r2;
                r2 = std::move(r1);
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_text() == "POST");
                BOOST_TEST(
                    r2.target() == "/x");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
            {
                request r1(cs);
                request r2(cs2);
                r2 = std::move(r1);
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_text() == "POST");
                BOOST_TEST(
                    r2.target() == "/x");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
            {
                request r1;
                request r2(cs);
                r2 = std::move(r1);
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r1.buffer().data() !=
                    r2.buffer().data());
                BOOST_TEST(
                    r2.method() == method::get);
                BOOST_TEST(
                    r2.method_text() == "GET");
                BOOST_TEST(
                    r2.version() == version::http_1_1);
            }
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
                request req;
                BOOST_TEST(req.capacity_in_bytes() == 0);
                req.clear();
                BOOST_TEST(req.capacity_in_bytes() == 0);
                BOOST_TEST(req.buffer() ==
                    "GET / HTTP/1.1\r\n\r\n");
            }
            {
                request req(
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
                request req;
                req.set_method(method::delete_);
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE / HTTP/1.1\r\n\r\n");
            }
            {
                request req(
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
                request req;
                req.set_method("DELETE");
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE / HTTP/1.1\r\n\r\n");
            }
            {
                request req(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("DELETE");
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_text() == "DELETE");
                BOOST_TEST(req.buffer() ==
                    "DELETE /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                request req(
                    "BOOST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("BOOST");
                BOOST_TEST(
                    req.method() == method::unknown);
                BOOST_TEST(
                    req.method_text() == "BOOST");
                BOOST_TEST(req.buffer() ==
                    "BOOST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_target
        {
            {
                request req;
                req.set_target("/index.htm");
                BOOST_TEST(
                    req.target() == "/index.htm");
                BOOST_TEST(req.buffer() ==
                    "GET /index.htm HTTP/1.1\r\n\r\n");
            }
            {
                request req(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_target("/index.htm");
                BOOST_TEST(req.buffer() ==
                    "POST /index.htm HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_version
        {
            {
                request req;
                req.set_version(version::http_1_0);
                BOOST_TEST(
                    req.version() == version::http_1_0);
                BOOST_TEST(req.buffer() ==
                    "GET / HTTP/1.0\r\n\r\n");
            }
            {
                request req(
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
        request req;
        req.set_expect_100_continue(true);
        BOOST_TEST(req.metadata().expect.is_100_continue == true);
        req.set_expect_100_continue(false);
        BOOST_TEST(req.metadata().expect.is_100_continue == false);
    }

    void
    run()
    {
        testHelpers();
        testSpecial();
        testObservers();
        testModifiers();
        testExpect();
    }
};

TEST_SUITE(
    request_test,
    "boost.http_proto.request");

} // http_proto
} // boost
