//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_view.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct natvis_test
{
    void
    run()
    {
        {
            request req(
                "GET / HTTP/1.1\r\n"
                "Connection: keep-alive\r\n"
                "Server: localhost\r\n"
                "\r\n");
            request_view rv(req);
        }
        {
            std::error_condition ec(std::errc::address_in_use);
            response res(
                "HTTP/1.1 200 OK\r\n"
                "Connection: keep-alive\r\n"
                "Server: localhost\r\n"
                "\r\n");
            response_view rv(res);
        }
    }
};

TEST_SUITE(natvis_test, "boost.http_proto.natvis");

} // http_proto
} // boost
