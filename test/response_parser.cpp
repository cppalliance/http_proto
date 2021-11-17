//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/response_parser.hpp>

#include <boost/http_proto/context.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class response_parser_test
{
public:
    void
    testBody()
    {
        context ctx;
        response_parser p(ctx);
        error_code ec;
#if 0
        p.commit(socket.read(p.prepare()));
        p.parse_header(ec);
        //...
        p.parse_body(..., ec); // ?
        // 1. body in parser-owned storage
        // 2. body in caller-owned storage
        // 3. clear existing parser-owned body storage
#endif
    }

    void
    run()
    {
    }
};

TEST_SUITE(response_parser_test, "boost.http_proto.response_parser");

} // http_proto
} // boost
