//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/range.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class range_test
{
public:
    struct bnf1
    {
        string_view value;

        char const*
        begin(
            char const* start,
            char const* end,
            error_code& ec) noexcept
        {
            (void)start;
            (void)end;
            (void)ec;
            return nullptr;
        }

        char const*
        increment(
            char const* start,
            char const* end,
            error_code& ec) noexcept
        {
            (void)start;
            (void)end;
            (void)ec;
            return nullptr;
        }
    };

    void
    run()
    {
    }
};

TEST_SUITE(range_test, "boost.http_proto.range");

} // http_proto
} // boost
