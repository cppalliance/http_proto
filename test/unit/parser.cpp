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

#include <vector>

namespace boost {
namespace http_proto {

struct parser_test
{
    using pieces = std::vector<
        core::string_view>;

    context ctx_;
    core::string_view sh_;
    core::string_view sb_;
    request_parser req_pr_;
    response_parser res_pr_;
    parser* pr_ = nullptr;

    parser_test()
        : ctx_()
        , req_pr_(
            [&]() -> context&
            {
                request_parser::config cfg;
                cfg.body_limit = 5;
                cfg.min_buffer = 3;
                install_parser_service(ctx_, cfg);
                return ctx_;
            }())
        , res_pr_(ctx_)
    {
    }

    struct test_sink : sink
    {
        std::string s;
        std::size_t max_size_;

        explicit
        test_sink(
            std::size_t max_size =
                std::size_t(-1)) noexcept
            : max_size_(max_size)
        {
        }

        results
        on_write(
            buffers::const_buffer b,
            bool more) noexcept override
        {
            (void)more;
            results rv;
            auto const space = 
                max_size_ - s.size();
            auto n = b.size();
            if( n > space)
                n = space;
            s.append(static_cast<
                char const*>(b.data()),
                    b.size());
            rv.bytes = n;
            if(n < b.size())
                rv.ec = error::buffer_overflow;
            return rv;
        }
    };

    //-------------------------------------------

    void
    read_some(
        pieces& in,
        system::error_code& ec)
    {
        if(! in.empty())
        {
            core::string_view& s = in[0];
            auto const n =
                buffers::buffer_copy(
                pr_->prepare(),
                buffers::buffer(
                    s.data(), s.size()));
            pr_->commit(n);
            s.remove_prefix(n);
            if(s.empty())
                in.erase(in.begin());
        }
        else
        {
            pr_->commit_eof();
        }
        pr_->parse(ec);
    }

    void
    read_header(
        pieces& in,
        system::error_code& ec)
    {
        while(! pr_->got_header())
        {
            read_some(in, ec);
            if(ec == condition::need_more_input)
                continue;
            if(ec.failed())
                return;
        }
        ec = {};
    }

    void
    read(
        pieces& in,
        system::error_code& ec)
    {
        if(pr_->is_complete())
        {
            pr_->parse(ec);
            return;
        }
        while(! pr_->is_complete())
        {
            read_some(in, ec);
            if(ec == condition::need_more_input)
                continue;
            if(ec.failed())
                return;
        }
    }

    //--------------------------------------------

    void
    start()
    {
        if(pr_->is_end_of_stream())
            pr_->reset();
        if(pr_ == &req_pr_)
            req_pr_.start();
        else
            res_pr_.start();
    }

    void
    check_in_place(
        pieces& in,
        system::error_code ex = {})
    {
        system::error_code ec;

        start();
        read_header(in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            return;
        }
        if(pr_->is_complete())
        {
            BOOST_TEST_EQ(pr_->body(), sb_);
            // this should be a no-op
            read(in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST_EQ(pr_->body(), sb_);
            return;
        }
        BOOST_TEST(pr_->body().empty());
        read(in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            return;
        }
        if(! BOOST_TEST(pr_->is_complete()))
            return;
        BOOST_TEST_EQ(pr_->body(), sb_);
        // this should be a no-op
        read(in, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(pr_->body(), sb_);
    }

    void
    check_dynamic(
        pieces& in,
        system::error_code ex = {})
    {
        char buf[5];
        system::error_code ec;

        start();
        read_header(in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            return;
        }
        auto& fb = pr_->set_body(
            buffers::flat_buffer(
                buf, sizeof(buf)));
        BOOST_TEST(pr_->body().empty());
        if(! pr_->is_complete())
        {
            read(in, ec);
            if(ec.failed())
            {
                BOOST_TEST_EQ(ec, ex);
                return;
            }
            if(! BOOST_TEST(pr_->is_complete()))
                return;
        }
        BOOST_TEST_EQ(
            test_to_string(fb.data()), sb_);
        BOOST_TEST(pr_->body().empty());
        // this should be a no-op
        read(in, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(
            test_to_string(fb.data()), sb_);
    }

    void
    check_sink(
        pieces& in,
        system::error_code ex = {})
    {
        system::error_code ec;

        start();
        read_header(in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            return;
        }
        auto& ts = pr_->set_body(test_sink{});
        BOOST_TEST(pr_->body().empty());
        if(! pr_->is_complete())
        {
            read(in, ec);
            if(ec.failed())
            {
                BOOST_TEST_EQ(ec, ex);
                return;
            }
            if(! BOOST_TEST(pr_->is_complete()))
                return;
        }
        BOOST_TEST_EQ(ts.s, sb_);
        BOOST_TEST(pr_->body().empty());
        // this should be a no-op
        read(in, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(ts.s, sb_);
    }

    //-------------------------------------------

    void
    check_header()
    {
        if(pr_ == &req_pr_)
        {
            request_view req;
            BOOST_TEST_NO_THROW((
                req = req_pr_.get()));
            BOOST_TEST_EQ(req.buffer(), sh_);
        }
        else
        {
            response_view res;
            BOOST_TEST_NO_THROW((
                res = res_pr_.get()));
            BOOST_TEST_EQ(res.buffer(), sh_);
        }
    }

    void
    check_req_1(
        pieces const& in0,
        system::error_code ex)
    {
        // in_place
        {
            auto in = in0;
            check_in_place(in, ex);
        }

        // dynamic
        {
            auto in = in0;
            check_dynamic(in, ex);
        }

        // sink
#if 0
        {
            auto in = in0;
            check_sink(in, ex);
        }
#endif
    }

    void
    check_res_1(
        pieces const& in0,
        system::error_code ex)
    {
        // in_place
        {
            auto in = in0;
            check_in_place(in, ex);
        }

        // dynamic
        {
            auto in = in0;
            check_dynamic(in, ex);
        }

        // sink
#if 0
        {
            auto in = in0;
            check_sink(
                res_pr_, in, ex);
        }
#endif
    }

    // void Fn( pieces& )
    template<class Fn>
    void
    grind(
        core::string_view sh,
        core::string_view sb,
        Fn const& fn)
    {
        std::string const s = [&]
        {
            std::string s;
            s.reserve(sh.size() + sb.size());
            s.append(sh.data(), sh.size());
            s.append(sb.data(), sb.size());
            return s;
        }();

        sh_ = sh;
        sb_ = sb;
        pieces in;
        in.reserve(3);
        core::string_view const sv(
            s.data(), s.size());

        // one piece
        in = { sv };
        fn(in);

        for(std::size_t i = 0;
            i <= s.size(); ++i)
        {
            // two pieces
            in = {
                sv.substr(0, i),
                sv.substr(i) };
            fn(in);

#if 0
            // VFALCO is this helpful?
            for(std::size_t j = i;
                j <= s.size(); ++j)
            {
                // three pieces
                in = {
                    sv.substr(0, i),
                    sv.substr(i, j - i),
                    sv.substr(j) };
                fn(in);
            }
#endif
        }
    }

    void
    check_req(
        core::string_view sh,
        core::string_view sb,
        system::error_code ex = {})
    {
        pr_ = &req_pr_;
        grind(sh, sb, [&](
            pieces const& in0)
            {
                check_req_1(in0, ex);
            });
    }

    void
    check_res(
        core::string_view sh,
        core::string_view sb,
        system::error_code ex = {})
    {
        pr_ = &res_pr_;
        grind(sh, sb, [&](
            pieces const& in0)
            {
                check_res_1(in0, ex);
            });
    }

    void
    should_pass(
        core::string_view sh,
        core::string_view sb)
    {
        BOOST_ASSERT(! sh.empty());
        if(sh[0] != 'H')
            check_req(sh, sb);
        else
            check_res(sh, sb);
    }

    void
    should_fail(
        system::error_code ex,
        core::string_view sh,
        core::string_view sb)
    {
        if(sh.empty())
        {
            BOOST_ASSERT(sb.empty());
            check_req(sh, sb, ex);
            check_res(sh, sb, ex);
            return;
        }
        if(sh[0] != 'H')
            check_req(sh, sb, ex);
        else
            check_res(sh, sb, ex);
    }

    //--------------------------------------------

    void
    testSpecial()
    {
        // ~parser

        {
            request_parser pr(ctx_);
        }

        {
            response_parser pr(ctx_);
        }

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
    testParseHeader()
    {
        // clean stream close
        should_fail(
            error::end_of_stream,
            "",
            "");

        // partial message
        should_fail(
            error::incomplete,
            "GET",
            "");

        // VFALCO TODO map grammar error codes
        // invalid syntax
        //should_fail(
            //error::bad_method,
            //" B", "");

        // payload error
        should_fail(
            error::bad_payload,
            "GET / HTTP/1.1\r\n"
            "Content-Length: 1\r\n"
            "Content-Length: 2\r\n"
            "\r\n",
            "12");

        should_pass(
            "GET / HTTP/1.1\r\n"
            "\r\n",
            "");
    }

    void
    testParseRequest()
    {
        should_pass(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "\r\n",
            "");

        should_pass(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "Content-Length: 3\r\n"
            "\r\n",
            "123");
    }

    void
    testParseResponse()
    {
        should_pass(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "");

        should_pass(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 3\r\n"
            "\r\n",
            "123");

        should_pass(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "123");

        should_fail(
            error::body_too_large,
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "Hello");
    }

    void
    testMembers()
    {
        //
        // start
        //

        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }

        {
            // start called twice
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }

        //
        // prepare
        //

        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.prepare(),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            BOOST_TEST_THROWS(
                pr.prepare(),
                std::logic_error);
        }

        //
        // commit
        //

        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.commit(0),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            BOOST_TEST_THROWS(
                pr.commit(0),
                std::logic_error);
        }

        {
            // n too large
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            BOOST_TEST_THROWS(
                pr.commit(1),
                std::invalid_argument);
        }

        {
            // n too large
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            auto dest = pr.prepare();
            BOOST_TEST_THROWS(
                pr.commit(buffers::buffer_size(dest) + 1),
                std::invalid_argument);
        }

        //
        // commit_eof
        //

        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.commit_eof(),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            BOOST_TEST_THROWS(
                pr.commit_eof(),
                std::logic_error);
        }

        //
        // parse
        //

        {
            // missing reset
            request_parser pr(ctx_);
            system::error_code ec;
            BOOST_TEST_THROWS(
                pr.parse(ec),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            system::error_code ec;
            pr.reset();
            BOOST_TEST_THROWS(
                pr.parse(ec),
                std::logic_error);
        }
    }

    //-------------------------------------------

    void
    run()
    {
        testSpecial();
        testConfig();
        //for(int i = 0; i < 10000; ++i )
        {
        testParseHeader();
        testParseRequest();
        testParseResponse();
        }
        testMembers();
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost
