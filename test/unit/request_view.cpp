//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_view.hpp>

#include <boost/http_proto/method.hpp>
#include <boost/http_proto/version.hpp>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

class request_view_test
{
public:
    void
    testView()
    {
        // request_view()
        {
            request_view req;
            BOOST_TEST(req.size() == 0);
            BOOST_TEST(req.method() == method::get);
            BOOST_TEST(req.method_str() == "GET");
            BOOST_TEST(req.target() == "/");
            BOOST_TEST(
                req.version() == version::http_1_1);
            BOOST_TEST(req.buffer() ==
                "GET / HTTP/1.1\r\n\r\n");
        }

        string_view s =
            "POST /x HTTP/1.0\r\n"
            "Content-Length: 42\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        // request_view(request_view)
        {
            {
                // default buffer
                request_view r1;
                request_view r2(r1);
                BOOST_TEST(r2.size() == 0);
                BOOST_TEST(r2.method() == method::get);
                BOOST_TEST(r2.method_str() == "GET");
                BOOST_TEST(r2.target() == "/");
                BOOST_TEST(
                    r2.version() == version::http_1_1);

            }
            {
                request_view r1 = make_request(s);
                BOOST_TEST(r1.method() == method::post);
                BOOST_TEST(r1.method_str() == "POST");
                BOOST_TEST(r1.target() == "/x");
                BOOST_TEST(
                    r1.version() == version::http_1_0);
                BOOST_TEST(
                    r1.buffer().data() == s.data());

                request_view r2(r1);
                BOOST_TEST(r2.size() == 2);;
                BOOST_TEST(r2.method() == method::post);
                BOOST_TEST(r2.method_str() == "POST");
                BOOST_TEST(r2.target() == "/x");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
                BOOST_TEST(
                    r2.buffer().data() == s.data());
            }
        }

        // operator=(request_view)
        {
            request_view r1 = make_request(s);
            request_view r2;
            r2 = r1;
            BOOST_TEST(r2.size() == 2);
            BOOST_TEST(r2.method() == method::post);
            BOOST_TEST(r2.method_str() == "POST");
            BOOST_TEST(r2.target() == "/x");
            BOOST_TEST(
                r2.version() == version::http_1_0);
            BOOST_TEST(
                r2.buffer().data() == s.data());
        }
    }

    void
    run()
    {
        testView();
    }
};

TEST_SUITE(
    request_view_test,
    "boost.http_proto.request_view");

} // http_proto
} // boost

