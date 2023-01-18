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

#include <boost/http_proto/codec.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class response_parser_test
{
public:
    void
    testSpecial()
    {
        // response_parser()
        response_parser();

        // response_parser(std::size_t)
        response_parser(4096);

        // response_parser(std::size_t, config)
        {
            response_parser::config cfg;
            cfg.max_header_size = 8192;
            response_parser(65536, cfg);
        }

        // response_parser(std::size_t, params)
        {
            response_parser(4096, gzip_decoder);
        }
    }

    void
    testStart()
    {
        response_parser(4096).start();

        response_parser(4096).start(headers_first);

        response_parser(4096).start(head_response);
    }

    void
    testBody()
    {
    }

    void
    run()
    {
        testSpecial();
        testStart();
    }
};

TEST_SUITE(
    response_parser_test,
    "boost.http_proto.response_parser");

} // http_proto
} // boost
