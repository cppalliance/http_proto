//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

/*
    Four body styles for `parser`
        * Specify a DynamicBuffer
        * Specify a Sink
        * Read from a parser::stream
        * in-place

    Four body styles for `serializer`
        * Specify a ConstBufferSequence
        * Specify a Source
        * Write into a serializer::stream
        * in-place

    struct half_duplex_client;
    struct full_duplex_client;
    struct pipelined_client;
    struct websocket_client;
    struct websocket_client_with_permessage_deflate;

    struct half_duplex_server;
    struct full_duplex_server;
    struct pipelined_server;
    struct websocket_server;
    struct websocket_server_with_permessage_deflate;

*/

struct sandbox_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    sandbox_test,
    "boost.http_proto.sandbox");

} // http_proto
} // boost
