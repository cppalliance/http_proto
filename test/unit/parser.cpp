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
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/service/zlib_service.hpp>
#include <boost/buffers/buffer.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <vector>

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
    using pieces = std::vector<string_view>;

    context ctx_;

    parser_test()
    {
        request_parser::config cfg;
        install_parser_service(ctx_, cfg);
    }

    static
    void
    feed(
        parser& pr,
        pieces& in,
        system::error_code& ec)
    {
        if(! in.empty())
        {
            string_view& s = in[0];
            auto const n =
                buffers::buffer_copy(
                pr.prepare(),
                buffers::buffer(
                    s.data(), s.size()));
            pr.commit(n);
            s.remove_prefix(n);
            if(s.empty())
                in.erase(in.begin());
        }
        else
        {
            pr.commit_eof();
        }
        pr.parse(ec);
    }

    void
    check_req_1(
        string_view sh,
        string_view sb,
        pieces& v)
    {
        request_parser pr(ctx_);
        pr.reset();
        pr.start();
        system::error_code ec;
        do
        {
            feed(pr, v, ec);
        }
        while(ec == condition::need_more_input);
        BOOST_TEST(pr.got_header());
        BOOST_TEST(pr.is_complete());
        BOOST_TEST_EQ(pr.get().buffer(), sh);
        if(sb.empty())
            return;
        BOOST_TEST_EQ(pr.in_place_body(), sb);
    }

    void
    check_res_1(
        string_view sh,
        string_view sb,
        pieces& v)
    {
        response_parser pr(ctx_);
        pr.reset();
        pr.start();
        system::error_code ec;
        do
        {
            feed(pr, v, ec);
        }
        while(ec == condition::need_more_input);
        BOOST_TEST(pr.got_header());
        BOOST_TEST(pr.is_complete());
        BOOST_TEST_EQ(pr.get().buffer(), sh);
        if(sb.empty())
            return;
        BOOST_TEST_EQ(pr.in_place_body(), sb);
    }

    void
    grind(
        string_view sh,
        string_view sb,
        void (parser_test::*mfn)(
            string_view, string_view, pieces&))
    {
        std::string const s = [&]
        {
            std::string s;
            s.reserve(sh.size() + sb.size());
            s.append(sh.data(), sh.size());
            s.append(sb.data(), sb.size());
            return s;
        }();
        pieces v;
        v.reserve(3);
        string_view const sv(
            s.data(), s.size());

        // one piece
        v = { sv };
        (this->*mfn)(sh, sb, v);

        for(std::size_t i = 0;
            i <= s.size(); ++i)
        {
            // two pieces
            v = {
                sv.substr(0, i),
                sv.substr(i) };
            (this->*mfn)(sh, sb, v);

            for(std::size_t j = i;
                j <= s.size(); ++j)
            {
                // three pieces
                v = {
                    sv.substr(0, i),
                    sv.substr(i, j - i),
                    sv.substr(j) };
                (this->*mfn)(sh, sb, v);
            }
        }
    }

    void
    check_req(
        string_view sh,
        string_view sb)
    {
        grind(sh, sb,
            &parser_test::check_req_1);
    }

    void
    check_res(
        string_view sh,
        string_view sb)
    {
        grind(sh, sb,
            &parser_test::check_res_1);
    }

    void
    testParseRequest()
    {
        check_req(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "\r\n",
            "");

        check_req(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "Content-Length: 5\r\n"
            "\r\n",
            "Hello");
    }

    void
    testParseResponse()
    {
        check_res(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "");

        check_res(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 5\r\n"
            "\r\n",
            "Hello");

        check_res(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "Hello");
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
    run()
    {
        testParseRequest();
        testParseResponse();
        testConfig();
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost
