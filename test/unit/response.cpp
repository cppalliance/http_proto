//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/response.hpp>

#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/field.hpp>

#include <boost/core/detail/string_view.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class response_test
{
public:
    static
    void
    check(
        response const& res,
        status sc,
        unsigned short si,
        core::string_view rs,
        version v)
    {
        BOOST_TEST_EQ(res.version(), v);
        BOOST_TEST_EQ(res.status(), sc);
        BOOST_TEST_EQ(res.status_int(), si);
        BOOST_TEST_EQ(res.reason(), rs);
    }

    void
    testSpecial()
    {
        // response(status, version)
        {
            {
                response res(status::ok);
                check(res, status::ok, 200, "OK", version::http_1_1);
                BOOST_TEST(res.capacity_in_bytes() == 0);
                BOOST_TEST_EQ(res.buffer(), "HTTP/1.1 200 OK\r\n\r\n");
            }

            {
                response res(status::ok, version::http_1_0);
                check(res, status::ok, 200, "OK", version::http_1_0);
                BOOST_TEST(res.capacity_in_bytes() > 0);
                BOOST_TEST_EQ(res.buffer(), "HTTP/1.0 200 OK\r\n\r\n");
            }

            {
                response res(status::not_found, version::http_1_0);
                check(res, status::not_found, 404, "Not Found", version::http_1_0);
                BOOST_TEST(res.capacity_in_bytes() > 0);
            }

            // same buffer
            {
                response r1(status::ok);
                response r2(status::ok);
                BOOST_TEST(r1.buffer().data() == r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() == 0);
                BOOST_TEST(r2.capacity_in_bytes() == 0);
            }

            // different buffer
            {
                response r1(status::not_found);
                response r2(status::not_found);
                BOOST_TEST(r1.buffer().data() != r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() > 0);
                BOOST_TEST(r2.capacity_in_bytes() > 0);
            }
        }

        // response()
        {
            {
                response res;
                check(res, status::ok, 200, "OK", version::http_1_1);
                BOOST_TEST(res.capacity_in_bytes() == 0);
            }

            // same buffer
            {
                response r1;
                response r2;
                BOOST_TEST(
                    r1.buffer().data() == r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() == 0);
                BOOST_TEST(r2.capacity_in_bytes() == 0);
            }
        }

        // response(response&&)
        {
            {
                response r1;
                response r2(std::move(r1));
                check(r1, status::ok, 200, "OK", version::http_1_1);
                check(r2, status::ok, 200, "OK", version::http_1_1);
                BOOST_TEST(
                    r1.buffer().data() == r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() == 0);
                BOOST_TEST(r2.capacity_in_bytes() == 0);
            }
            {
                response r1(status::not_found, version::http_1_0);
                response r2(std::move(r1));
                check(r1, status::ok, 200, "OK", version::http_1_1);
                check(r2, status::not_found, 404, "Not Found", version::http_1_0);
                BOOST_TEST(
                    r1.buffer().data() != r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() == 0);
                BOOST_TEST(r2.capacity_in_bytes() != 0);
            }
        }

        // response(response const&)
        {
            {
                response r1;
                response r2(r1);
                check(r1, status::ok, 200, "OK", version::http_1_1);
                check(r2, status::ok, 200, "OK", version::http_1_1);
                BOOST_TEST(
                    r1.buffer().data() == r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() == 0);
                BOOST_TEST(r2.capacity_in_bytes() == 0);
            }
            {
                response r1(status::not_found, version::http_1_0);
                response r2(r1);
                check(r1, status::not_found, 404, "Not Found", version::http_1_0);
                check(r2, status::not_found, 404, "Not Found", version::http_1_0);
                BOOST_TEST(
                    r1.buffer().data() != r2.buffer().data());
                BOOST_TEST(r1.capacity_in_bytes() > 0);
                BOOST_TEST(r2.capacity_in_bytes() > 0);
            }
        }

        // operator=(response&&)
        {
            response r1;
            response r2(status::not_found, version::http_1_0);
            r1 = std::move(r2);
            check(r1, status::not_found, 404, "Not Found", version::http_1_0);
            check(r2, status::ok, 200, "OK", version::http_1_1);
        }

        // operator=(response const&)
        {
            response r1;
            response r2(status::not_found, version::http_1_0);
            r1 = r2;
            check(r1, status::not_found, 404, "Not Found", version::http_1_0);
            check(r2, status::not_found, 404, "Not Found", version::http_1_0);
            BOOST_TEST(
                r1.buffer().data() != r2.buffer().data());
            BOOST_TEST(r1.capacity_in_bytes() > 0);
            BOOST_TEST(r2.capacity_in_bytes() > 0);
        }

        //----------------------------------------

        // response(response_view const&)
        {
            core::string_view const s =
                "HTTP/1.0 404 Not Found\r\n"
                "Server: test\r\n"
                "\r\n";
            response r(s);
            response_view rv = r;
            response res(rv);
            check(res, status::not_found, 404, "Not Found", version::http_1_0);
            BOOST_TEST_EQ(res.buffer(), s);
            BOOST_TEST(res.buffer().data() != s.data());
            BOOST_TEST(res.begin()->id == field::server);
            BOOST_TEST(res.begin()->name == "Server");
            BOOST_TEST(res.begin()->value == "test");
        }

        // operator=(response_view const&)
        {
            core::string_view const s =
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Server: test\r\n"
                "\r\n";
            response r(s);
            response_view rv = r;
            response res(status::not_found);
            res = rv;
            BOOST_TEST_EQ(res.buffer(), s);
            BOOST_TEST(res.buffer().data() != s.data());
            check(res, status::switching_protocols, 101, "Switching Protocols", version::http_1_1);
            BOOST_TEST(res.begin()->id == field::server);
            BOOST_TEST(res.begin()->name == "Server");
            BOOST_TEST(res.begin()->value == "test");
        }

        //----------------------------------------

        // operator response_view()
        {
            {
                response res;
                response_view rv(res);
                BOOST_TEST_EQ(rv.version(), version::http_1_1);
                BOOST_TEST_EQ(rv.status(), status::ok);
                BOOST_TEST_EQ(rv.status_int(), 200);
                BOOST_TEST_EQ(rv.reason(), "OK");
                BOOST_TEST_EQ(rv.buffer(), "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST(rv.buffer().data() == res.buffer().data());
            }
            {
                response res(status::not_found, version::http_1_0);
                response_view rv(res);
                BOOST_TEST_EQ(rv.version(), version::http_1_0);
                BOOST_TEST_EQ(rv.status(), status::not_found);
                BOOST_TEST_EQ(rv.status_int(), 404);
                BOOST_TEST_EQ(rv.reason(), "Not Found");
                BOOST_TEST_EQ(rv.buffer(), "HTTP/1.0 404 Not Found\r\n\r\n");
                BOOST_TEST(rv.buffer().data() == res.buffer().data());
            }
        }
    }

    void
    testModifiers()
    {
        // clear()
        {
            {
                response res;
                BOOST_TEST(res.capacity_in_bytes() == 0);
                res.clear();
                BOOST_TEST(res.buffer() == "HTTP/1.1 200 OK\r\n\r\n");
            }
            {
                response res(status::not_found, version::http_1_0);
                BOOST_TEST(res.capacity_in_bytes() > 0);
                res.clear();
                check(res, status::ok, 200, "OK", version::http_1_1);
                BOOST_TEST(res.capacity_in_bytes() > 0);
            }
        }

        // set_start_line()
        {
            {
                response res;
                res.set_start_line(status::not_found);
                check(res, status::not_found, 404, "Not Found", version::http_1_1);
            }
            {
                response res;
                res.set_start_line(status::switching_protocols, version::http_1_0);
                check(res, status::switching_protocols, 101, "Switching Protocols", version::http_1_0);
            }
            {
                response res;
                res.set_start_line(199, "Huh", version::http_1_1);
                check(res, status::unknown, 199, "Huh", version::http_1_1);
            }
            {
                response res;
                res.set_start_line(199, "Huh", version::http_1_1);
                check(res, status::unknown, 199, "Huh", version::http_1_1);

                res.set_start_line(199, "Huh", version::http_1_1);
                check(res, status::unknown, 199, "Huh", version::http_1_1);

                res.set_start_line(199, "ab", version::http_1_1);
                check(res, status::unknown, 199, "ab", version::http_1_1);

                res.set_start_line(199, "a", version::http_1_1);
                check(res, status::unknown, 199, "a", version::http_1_1);

                res.set_start_line(199, "abcdefghijklmnopqrstuvwxyz", version::http_1_1);
                check(res, status::unknown, 199, "abcdefghijklmnopqrstuvwxyz", version::http_1_1);
            }
            {
                core::string_view s =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                response r(s);
                response_view rv = r;
                response res(rv);
                check(res, status::ok, 200, "OK", version::http_1_1);
                BOOST_TEST(res.size() == 2);
                auto it = res.begin();
                BOOST_TEST_EQ(it->id, field::server);
                BOOST_TEST_EQ(it->name, "Server");
                BOOST_TEST_EQ(it->value, "test");
                ++it;
                BOOST_TEST_EQ(it->id, field::content_length);
                BOOST_TEST_EQ(it->name, "Content-Length");
                BOOST_TEST_EQ(it->value, "0");
            }
        }
    }

    void
    run()
    {
        testSpecial();
        testModifiers();
    }
};

TEST_SUITE(
    response_test,
    "boost.http_proto.response");

} // http_proto
} // boost
