//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/parser.hpp>

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/service/zlib_service.hpp>
#include <boost/buffers/buffer.hpp>
#include <boost/buffers/buffer_copy.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

/*
    Four body styles for `parser`
        * Specify a DynamicBuffer
        * Specify a Sink
        * Read from a parser::stream
        * in-place
*/
struct parser_test
{
    static
    result<request_view>
    read_header(
        string_view& in,
        request_parser& pr)
    {
        result<request_view> rv;
        auto n = buffers::buffer_copy(
            pr.prepare(),
            buffers::buffer(
                in.data(),
                in.size()));
        in.remove_prefix(n);
        return rv;
    }

    static
    void
    feed(
        parser& pr,
        string_view& s,
        std::size_t n,
        error_code& ec)
    {
        auto dest = pr.prepare();
        if( n > s.size())
            n = s.size();
        auto const n1 = buffers::buffer_copy(
            dest, buffers::const_buffer(
                s.data(), n));
        BOOST_TEST_EQ(n1, n);
        pr.commit(n1);
        s = s.substr(n);
        pr.parse(ec);
    }

    void
    testConfig()
    {
    #ifdef BOOST_HTTP_PROTO_HAS_ZLIB
        context ctx;

        zlib::deflate_decoder_service::config cfg0;
        cfg0.install(ctx);

        request_parser::config_base cfg1;
        cfg1.apply_deflate_decoder = true;
        install_parser_service(ctx, cfg1);
    #endif
    }

    void
    testParse()
    {
        context ctx;
        request_parser::config cfg;
        install_parser_service(ctx, cfg);

        auto const check =
        [&ctx](string_view const s0)
        {
            request_parser pr(ctx);
            for(std::size_t i = 1;
                i < s0.size() - 1; ++i)
            {
                auto s = s0;
                pr.start();
                for(;;)
                {
                    error_code ec;
                    feed(pr, s, i, ec);
                    if(ec == grammar::error::need_more)
                        continue;
                    if(! BOOST_TEST(! ec.failed()))
                    {
                        pr.reset();
                        break;
                    }
                    BOOST_TEST(pr.got_header());
                    BOOST_TEST_EQ(
                        pr.get().buffer(), s0);
                    break;
                }
            }
        };

        check(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "\r\n");
    }

    void
    run()
    {
        testConfig();
        testParse();
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost
