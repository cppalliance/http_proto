//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/headers.hpp>

#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/response.hpp>
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class headers_test
{
public:
    void
    testSpecial()
    {
        // default ctor
        {
            headers h;
            BOOST_TEST(h.begin() == h.end());
            BOOST_TEST(h.size() == 0);
            BOOST_TEST(
                h.get_const_buffer() == "\r\n");
            BOOST_TEST(
                h.capacity_in_bytes() == 0);
        }

        // move ctor
        {
            // no start line
            {
                headers h1;
                h1.append("x", "1");
                BOOST_TEST(h1.size() == 1);
                BOOST_TEST(
                    h1.capacity_in_bytes() > 0);
                BOOST_TEST(h1.at("x") == "1");
                headers h2(std::move(h1));
                BOOST_TEST(
                    h1.capacity_in_bytes() == 0);
                BOOST_TEST(h1.size() == 0);
                BOOST_TEST(! h1.exists("x"));
                BOOST_TEST(h2.size() == 1);
                BOOST_TEST(h2.at("x") == "1");
                BOOST_TEST(
                    h2.capacity_in_bytes() > 0);
            }

            // start line
            {
                response res;
                res.set_result(
                    status::bad_request,
                    version::http_1_1);
                BOOST_TEST(res.get_const_buffer() ==
                    "HTTP/1.1 400 Bad Request\r\n\r\n");
                res.fields.append("x", "1");
                headers h(std::move(res.fields));
                BOOST_TEST(res.get_const_buffer() ==
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST(res.fields.size() == 0);
                // start-line is removed on move
                BOOST_TEST(h.get_const_buffer() ==
                    "x: 1\r\n\r\n");
                BOOST_TEST(h.size() == 1);
                BOOST_TEST(h.count("x") == 1);
            }
        }

        // copy ctor
        {
            // no buffer
            {
                headers h1;
                BOOST_TEST(h1.size() == 0);
                BOOST_TEST(h1.get_const_buffer()
                    == "\r\n");
                BOOST_TEST(
                    h1.capacity_in_bytes() == 0);
                headers h2(h1);
                BOOST_TEST(h2.size() == 0);
                BOOST_TEST(h2.get_const_buffer()
                    == "\r\n");
                BOOST_TEST(
                    h2.capacity_in_bytes() == 0);
            }

            // no start-line
            {
                headers h1;
                h1.append("x", "1");
                BOOST_TEST(
                    h1.capacity_in_bytes() > 0);
                BOOST_TEST(h1.size() == 1);
                BOOST_TEST(h1.at("x") == "1");
                headers h2(h1);
                BOOST_TEST(
                    h1.capacity_in_bytes() > 0);
                BOOST_TEST(
                    h2.capacity_in_bytes() > 0);
                BOOST_TEST(h1.size() == 1);
                BOOST_TEST(h1.at("x") == "1");
                BOOST_TEST(h2.size() == 1);
                BOOST_TEST(h2.at("x") == "1");
                BOOST_TEST(h2.count("x") == 1);
            }

            // start line
            {
                response res;
                res.set_result(
                    status::bad_request,
                    version::http_1_1);
                BOOST_TEST(res.get_const_buffer() ==
                    "HTTP/1.1 400 Bad Request\r\n\r\n");
                res.fields.append("x", "1");
                headers h(res.fields);
                BOOST_TEST(res.get_const_buffer() ==
                    "HTTP/1.1 400 Bad Request\r\nx: 1\r\n\r\n");
                BOOST_TEST(res.fields.size() == 1);
                // start-line is removed on move
                BOOST_TEST(h.get_const_buffer() ==
                    "x: 1\r\n\r\n");
                BOOST_TEST(h.size() == 1);
                BOOST_TEST(h.count("x") == 1);
            }
        }

        // copy assign
        {
            // empty container
            {
                headers h1;
                BOOST_TEST(h1.size() == 0);
                BOOST_TEST(
                    h1.capacity_in_bytes() == 0);
                headers h2;
                h2 = h1;
                BOOST_TEST(h2.size() == 0);
                BOOST_TEST(
                    h1.capacity_in_bytes() == 0);
                BOOST_TEST(
                    h2.capacity_in_bytes() == 0);
            }

            // keep capacity
            {
                headers h1;
                h1.append("x", "1");
                BOOST_TEST(
                    h1.capacity_in_bytes() > 0);
                headers h2;
                h1 = h2;
                BOOST_TEST(
                    h1.capacity_in_bytes() > 0);
                BOOST_TEST(h1.size() == 0);
                BOOST_TEST(
                    h1.begin() == h1.end());
            }

            // use existing capacity
            {
                headers h1;
                h1.append("x", "1");
                h1.append("y", "2");
                h1.append("z", "3");
                headers h2;
                h2.append("a", "1");
                BOOST_TEST(
                    h1.capacity_in_bytes() >=
                    h2.capacity_in_bytes());
                h1 = h2;
                BOOST_TEST(h1.size() == 1);
                BOOST_TEST(h1.count("a") == 1);
                BOOST_TEST(h1.count("x") == 0);
                BOOST_TEST(h1.count("y") == 0);
                BOOST_TEST(h1.count("z") == 0);
            }

            // realize start-line
            {
                response res;
                BOOST_TEST(
                    res.fields.capacity_in_bytes() == 0);
                headers h;
                h.append("x", "1");
                res.fields = h;
                BOOST_TEST(res.get_const_buffer() ==
                    "HTTP/1.1 200 OK\r\nx: 1\r\n\r\n");
            }

            // force allocation
            {
                headers h1;
                h1.append("x", "1");
                h1.append("y", "2");
                h1.append("z", "3");
                headers h2;
                h2.append("a",
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789"
                    "01234567890123456789012345678901234567890123456789");
                BOOST_TEST(
                    h1.capacity_in_bytes() <
                    h2.capacity_in_bytes());
                h1 = h2;
                BOOST_TEST(h1.size() == 1);
                BOOST_TEST(h1.count("a") == 1);
                BOOST_TEST(h1.count("x") == 0);
                BOOST_TEST(h1.count("y") == 0);
                BOOST_TEST(h1.count("z") == 0);
            }
        }
    }

    void
    testObservers()
    {
        // operator header_view
        {
            // no start line
            {
                headers h;
                h.append("x", "1");
                h.append("y", "2");
                fields_view fv = h;
                BOOST_TEST(fv.count("x") == 1);
                BOOST_TEST(fv.count("y") == 1);
                BOOST_TEST(fv.get_const_buffer() ==
                    "x: 1\r\ny: 2\r\n\r\n");
            }

            // start line
            {
                response res;
                res.fields.append("x", "1");
                res.fields.append("y", "2");
                fields_view f = res.fields;
                BOOST_TEST(f.get_const_buffer() ==
                    "HTTP/1.1 200 OK\r\nx: 1\r\ny: 2\r\n\r\n");
                BOOST_TEST(f.count("x") == 1);
                BOOST_TEST(f.count("y") == 1);
            }
        }
    }

    void
    testAppend()
    {
        headers h;
        h.append(field::user_agent, "x");
        h.append(field::connection, "close");
        h.append(field::transfer_encoding, "chunked");
        h.append("a", "1" );
        h.append("b", "2" );
        h.append("a", "3" );
        h.append("c", "4" );

        string_view s =
            "User-Agent: x\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "a: 1\r\n"
            "b: 2\r\n"
            "a: 3\r\n"
            "c: 4\r\n"
            "\r\n";
        BOOST_TEST(h.get_const_buffer() == s);
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

        auto it = h.begin();
        BOOST_TEST(it->id == field::user_agent);
        BOOST_TEST(it->name == "User-Agent");
        BOOST_TEST(it->value == "x");
        ++it;
        BOOST_TEST(it->id == field::connection);
        BOOST_TEST(it->name == "Connection");
        BOOST_TEST(it->value == "close");
        ++it;
        BOOST_TEST(it->id == field::transfer_encoding);
        BOOST_TEST(it->name == "Transfer-Encoding");
        BOOST_TEST(it->value == "chunked");
        ++it;
        BOOST_TEST(it->id == field::unknown);
        BOOST_TEST(it->name == "a");
        BOOST_TEST(it->value == "1");
        ++it;
        BOOST_TEST(it->id == field::unknown);
        BOOST_TEST(it->name == "b");
        BOOST_TEST(it->value == "2");
        ++it;
        BOOST_TEST(it->id == field::unknown);
        BOOST_TEST(it->name == "a");
        BOOST_TEST(it->value == "3");
        ++it;
        BOOST_TEST(it->id == field::unknown);
        BOOST_TEST(it->name == "c");
        BOOST_TEST(it->value == "4");
        ++it;
    }

    void
    testConversion()
    {
        auto const f =
            [](fields_view){};
        headers h;
        // must compile
        f(h);
    }

    void run()
    {
        testSpecial();
        testObservers();
        testAppend();
    }
};

TEST_SUITE(
    headers_test,
    "boost.http_proto.headers");

} // http_proto
} // boost
