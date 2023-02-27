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
#include <boost/buffers/flat_buffer.hpp>
#include <vector>

#include "test_helpers.hpp"

#include <future>

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
    using pieces = std::vector<
        core::string_view>;

    context ctx_;

    parser_test()
    {
        request_parser::config cfg;
        install_parser_service(ctx_, cfg);
    }

    //-------------------------------------------

    static
    void
    read_some(
        pieces& in,
        parser& pr,
        system::error_code& ec)
    {
        if(! in.empty())
        {
            core::string_view& s = in[0];
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

    static
    void
    read_header(
        pieces& in,
        parser& pr,
        system::error_code& ec)
    {
        while(! pr.got_header())
        {
            read_some(in, pr, ec);
            if(ec == condition::need_more_input)
                continue;
            if(ec.failed())
                return;
        }
        ec = {};
    }

    static
    void
    read(
        pieces& in,
        parser& pr,
        system::error_code& ec)
    {
        if(pr.is_complete())
        {
            pr.parse(ec);
            return;
        }
        while(! pr.is_complete())
        {
            read_some(in, pr, ec);
            if(ec == condition::need_more_input)
                continue;
            if(ec.failed())
                return;
        }
    }

    //-------------------------------------------

    void
    check_in_place(
        core::string_view sb,
        parser& pr,
        pieces& in,
        system::error_code expected = {})
    {
        system::error_code ec;
        read_header(in, pr, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, expected);
            return;
        }
        if(pr.is_complete())
        {
            BOOST_TEST_EQ(pr.body(), sb);
            // this should be a no-op
            read(in, pr, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST_EQ(pr.body(), sb);
            return;
        }
        BOOST_TEST(pr.body().empty());
        read(in, pr, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, expected);
            return;
        }
        if(! BOOST_TEST(pr.is_complete()))
            return;
        BOOST_TEST_EQ(pr.body(), sb);
        // this should be a no-op
        read(in, pr, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(pr.body(), sb);
    }

    void
    check_dynamic(
        core::string_view sb,
        parser& pr,
        pieces& in,
        system::error_code expected = {})
    {
        char buf[5];
        system::error_code ec;
        read_header(in, pr, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, expected);
            return;
        }
        auto& fb = pr.set_body(
            buffers::flat_buffer(
                buf, sizeof(buf)));
        BOOST_TEST(pr.body().empty());
        if(! pr.is_complete())
        {
            read(in, pr, ec);
            if(ec.failed())
            {
                BOOST_TEST_EQ(ec, expected);
                return;
            }
            if(! BOOST_TEST(pr.is_complete()))
                return;
        }
        BOOST_TEST_EQ(
            test_to_string(fb.data()), sb);
        BOOST_TEST(pr.body().empty());
        // this should be a no-op
        read(in, pr, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(
            test_to_string(fb.data()), sb);
    }

    //-------------------------------------------

    template<class Parser>
    void
    start(Parser& pr)
    {
        if(pr.is_end_of_stream())
            pr.reset();
        pr.start();
    }

    void
    check_req_1(
        request_parser& pr,
        pieces& in0,
        core::string_view sh,
        core::string_view sb,
        system::error_code expected)
    {
        (void)sh;

        // in_place
        {
            auto in = in0;
            start(pr);
            check_in_place(
                sb, pr, in, expected);
        }

        // dynamic
        {
            auto in = in0;
            start(pr);
            check_dynamic(
                sb, pr, in, expected);
        }
    }

    void
    check_res_1(
        response_parser& pr,
        pieces& in0,
        core::string_view sh,
        core::string_view sb,
        system::error_code expected)
    {
        (void)sh;

        // in_place
        {
            auto in = in0;
            start(pr);
            check_in_place(
                sb, pr, in, expected);
        }

        // dynamic
        {
            auto in = in0;
            start(pr);
            check_dynamic(
                sb, pr, in, expected);
        }
    }

    template<class Parser>
    void
    grind(
        Parser& pr,
        core::string_view sh,
        core::string_view sb,
        system::error_code expected,
        void (parser_test::*mfn)(
            Parser& pr,
            pieces&,
            core::string_view,
            core::string_view,
            system::error_code))
    {
        std::string const s = [&]
        {
            std::string s;
            s.reserve(sh.size() + sb.size());
            s.append(sh.data(), sh.size());
            s.append(sb.data(), sb.size());
            return s;
        }();
        pieces in;
        in.reserve(3);
        core::string_view const sv(
            s.data(), s.size());

        // one piece
        in = { sv };
        (this->*mfn)(
            pr, in, sh, sb, expected);

        for(std::size_t i = 0;
            i <= s.size(); ++i)
        {
            // two pieces
            in = {
                sv.substr(0, i),
                sv.substr(i) };
            (this->*mfn)(
                pr, in, sh, sb, expected);

            for(std::size_t j = i;
                j <= s.size(); ++j)
            {
                // three pieces
                in = {
                    sv.substr(0, i),
                    sv.substr(i, j - i),
                    sv.substr(j) };
                (this->*mfn)(
                    pr, in, sh, sb, expected);
            }
        }
    }

    void
    check_req(
        core::string_view sh,
        core::string_view sb,
        system::error_code expected = {})
    {
        request_parser pr(ctx_);
        pr.reset();
        grind(pr, sh, sb, expected,
            &parser_test::check_req_1);
    }

    void
    check_res(
        core::string_view sh,
        core::string_view sb,
        system::error_code expected = {})
    {
        response_parser pr(ctx_);
        pr.reset();
        grind(pr, sh, sb, expected,
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

        check_res(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "Hellox",
            error::buffer_overflow);
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
        //for(int i = 0; i < 10000; ++i )
        {
        testParseRequest();
        testParseResponse();
        testConfig();
        }
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost
