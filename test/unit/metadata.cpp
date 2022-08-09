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
            request const& req,
            content_length cl1)
        {
            auto const cl0 =
                req.content_length();
            BOOST_TEST_EQ(
                cl0.count, cl1.count);
            BOOST_TEST_EQ(
                cl0.value, cl1.value);
            BOOST_TEST_EQ(
                cl0.has_value, cl1.has_value);
        };

        //
        request req;
        check(req, { 0, 0, false });

        // Content-Length: 0
        req.append(field::content_length, "0");
        check(req, { 1, 0, true });

        // Content-Length: 1
        req.set(field::content_length, "1");
        check(req, { 1, 1, true });

        // Content-Length: 1
        // Content-Length: 1
        req.append(field::content_length, "1");
        check(req, { 2, 1, true });

        //
        req.erase(field::content_length);
        check(req, { 0, 0, false });

        // Content-Length: 2
        req.set(field::content_length, "2");
        check(req, { 1, 2, true });

        // Content-Length: 2
        // Content-Length: 3
        req.append(field::content_length, "3");
        check(req, { 2, 0, false });

        // Content-Length: 3
        req.erase(req.find(field::content_length));
        check(req, { 1, 3, true });

        // Content-Length: 42
        req.set_content_length(42);
        check(req, { 1, 42, true });

        // Content-Length: 0
        req.set_content_length(0);
        check(req, { 1, 0, true });

        // Content-Length: 18446744073709551616
        req.set(field::content_length,
            "18446744073709551616");
        check(req, { 1, 0, false });

        // Content-Length: 18446744073709551616
        // Content-Length: 42
        req.append(field::content_length, "42");
        check(req, { 2, 0, false });

        // Content-Length: 42
        req.erase(req.find(field::content_length));
        check(req, { 1, 42, true });
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
