//
// Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/request.hpp>

#include "test_suite.hpp"

#include <string>

namespace boost {
namespace http_proto {

// This has to be at least 19 because
// of the default request buffer.
static_assert(max_off_t == 20, "max_off_t != 20");

class limits_test
{
public:
    void
    testLimits()
    {
        {
            request req;
            BOOST_TEST_THROWS(
                req.set_start_line(
                    method::connect,
                    "https://www.example.com",
                    version::http_1_1),
                std::length_error);
        }
    }

    void
    testFields()
    {
        // reserve
        {
            fields f;
            BOOST_TEST_THROWS(
                f.reserve(max_off_t + 1),
                std::length_error);
        }

        // append
        {
            fields f;
            std::string s;
            s.append(max_off_t, 'x');
            BOOST_TEST_THROWS(
                f.append(s, "v"),
                std::length_error);
        }

        // set
        {
            fields f;
            BOOST_TEST_THROWS(
                f.set("x",
                    "0123456789"
                    "0123456789"),
                std::length_error);
        }
    }

    void
    run()
    {
        testLimits();
        testFields();
    }
};

TEST_SUITE(
    limits_test,
    "boost.http_proto.limits");

} // http_proto
} // boost
