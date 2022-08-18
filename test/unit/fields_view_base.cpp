//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_view_base.hpp>

#include "test_helpers.hpp"

#include <boost/url/grammar/range_rule.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>

namespace boost {
namespace http_proto {

struct fields_view_base_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    fields_view_base_test,
    "boost.http_proto.fields_view_base");

} // http_proto
} // boost
