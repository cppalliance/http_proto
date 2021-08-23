//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request.hpp>

#include <boost/http_proto/method.hpp>
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class request_test
{
public:
    void
    run()
    {
        // default-ctor
        {
            request req;
            BOOST_TEST(req.str() ==
                "GET / HTTP/1.1\r\n\r\n");
            BOOST_TEST(
                req.method() == method::get);
            BOOST_TEST(
                req.method_str() == "GET");
            BOOST_TEST(
                req.target() == "/");
            BOOST_TEST(
                req.version() == version::http_1_1);
        }

        // set_method
        {
            request req;
            req.set_method(method::delete_);
            BOOST_TEST(
                req.method() == method::delete_);
            BOOST_TEST(
                req.method_str() == "DELETE");
            BOOST_TEST(req.str() ==
                "DELETE / HTTP/1.1\r\n\r\n");
        }

        // set_method
        {
            request req;

            req.set_method("POST");
            BOOST_TEST(
                req.method() == method::post);
            BOOST_TEST(
                req.method_str() == "POST");
            BOOST_TEST(req.str() ==
                "POST / HTTP/1.1\r\n\r\n");

            req.set_method("BOOST");
            BOOST_TEST(
                req.method() == method::unknown);
            BOOST_TEST(
                req.method_str() == "BOOST");
            BOOST_TEST(req.str() ==
                "BOOST / HTTP/1.1\r\n\r\n");
        }

        // set_method
        {
            request req;

            req.set_method("POST");
        }
    }
};

TEST_SUITE(
    request_test,
    "boost.http_proto.request");

} // http_proto
} // boost

