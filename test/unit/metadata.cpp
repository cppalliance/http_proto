//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/metadata.hpp>

#include <boost/http_proto/request.hpp>

#include "test_helpers.hpp"

#include <utility>

namespace boost {
namespace http_proto {

// These tests check that the message
// containers correctly track changes
// in the metadata

struct metadata_test
{
    void
    testContentLength()
    {
        auto const check = [](
            string_view s,
            void(*f)(message_base&),
            metadata::content_length_t cl1)
        {
            request req = make_request_(s);
            f(req);
            auto const cl0 =
                req.metadata().content_length;
            BOOST_TEST_EQ(
                cl0.ec, cl1.ec);
            BOOST_TEST_EQ(
                cl0.count, cl1.count);
            BOOST_TEST_EQ(
                cl0.has_value, cl1.has_value);
            BOOST_TEST_EQ(
                cl0.value, cl1.value);
        };

        check(
            "GET / HTTP/1.1\r\n"
            "\r\n",
            [](message_base&)
            {
            },
            { error::success, 0, false, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.append(field::content_length, "0");
            },
            { error::success, 1, true, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.set(field::content_length, "1");
            },
            { error::success, 1, true, 1 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 1\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.append(field::content_length, "1");
            },
            { error::success, 2, true, 1 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 1\r\n"
            "Content-Length: 1\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.erase(field::content_length);
            },
            { error::success, 0, false, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.set(field::content_length, "2");
            },
            { error::success, 1, true, 2 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 2\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.append(field::content_length, "3");
            },
            { error::multiple_content_length, 2, false, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 2\r\n"
            "Content-Length: 3\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.erase(f.find(field::content_length));
            },
            { error::success, 1, true, 3 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 3\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.set_content_length(42);
            },
            { error::success, 1, true, 42 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 42\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.set_content_length(0);
            },
            { error::success, 1, true, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.set(field::content_length,
                    "18446744073709551616");
            },
            //{ grammar::error::invalid, 1, false, 0 });
            { error::bad_content_length, 1, false, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 18446744073709551616\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.append(field::content_length, "42");
            },
            //{ grammar::error::invalid, 2, false, 0 });
            { error::bad_content_length, 2, false, 0 });

        check(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 18446744073709551616\r\n"
            "Content-Length: 42\r\n"
            "\r\n",
            [](message_base& f)
            {
                f.erase(f.find(field::content_length));
            },
            { error::success, 1, true, 42 });
    }

    void
    run()
    {
        testContentLength();
    }
};

TEST_SUITE(
    metadata_test,
    "boost.http_proto.metadata");

} // http_proto
} // boost
