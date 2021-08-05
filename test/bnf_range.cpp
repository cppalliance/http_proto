//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf_range.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class bnf_range_test
{
public:
    struct bnf1
    {
#if 0
        string_view value;
        static char const* begin(
            state& st,
            char const* start,
            char const* end,
            error_code& ec) noexcept;
#endif
    };

    void
    run()
    {
    }
};

TEST_SUITE(bnf_range_test, "boost.http_proto.bnf_range");

} // http_proto
} // boost
