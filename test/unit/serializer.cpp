//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/serializer.hpp>

#include <boost/http_proto/response.hpp>
#include <boost/http_proto/string_body.hpp>
#include <boost/core/ignore_unused.hpp>

#include "test_helpers.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct serializer_test
{
    struct test_source : source
    {
        test_source()
            : s_("12345")
        {
        }

        void
        maybe_reserve(
            std::size_t limit,
            reserve_fn const& reserve) override
        {
            ignore_unused(limit, reserve);
        }

        results
        read(
            mutable_buffers_pair dest) override
        {
            results rv;
            rv.bytes = buffer_copy(
                dest,
                const_buffer(
                    s_.data(),
                    s_.size()));
            s_ = s_.substr(rv.bytes);
            rv.more = ! s_.empty();
            rv.ec = {};
            return rv;
        }

    private:
        string_view s_;
    };

    template<
        class ConstBuffers>
    static
    std::size_t
    append(
        std::string& dest,
        ConstBuffers const& src)
    {
        auto n0 = dest.size();
        auto n = buffer_size(src);
        dest.resize(n0 + n);
        buffer_copy(mutable_buffer(
            &dest[n0], n), src);
        return n;
    }

    static
    std::string
    read_some(serializer& sr)
    {
        std::string s;
        auto cbs = sr.prepare().value();
        s.resize(buffer_size(cbs));
        buffer_copy(mutable_buffer(
            &s[0], s.size()), cbs);
        sr.consume(s.size());
        return s;
    }

    static
    std::string
    read(serializer& sr)
    {
        std::string s;
        while(! sr.is_done())
            s += read_some(sr);
        return s;
    }

    template<class T>
    static
    T const&
    make_const(T&& t) noexcept
    {
        return t;
    }

    //--------------------------------------------

    void
    testSyntax()
    {
        serializer sr(1024);
        response res;

        sr.reset(res);
        sr.reset(res, const_buffer{});
        sr.reset(res, mutable_buffer{});
        sr.reset(res, const_buffers_1{});
        sr.reset(res, mutable_buffers_1{});
        sr.reset(res, test_source{});
        sr.reset(res, make_const(const_buffer{}));
        sr.reset(res, make_const(mutable_buffer{}));
        sr.reset(res, make_const(const_buffers_1{}));
        sr.reset(res, make_const(mutable_buffers_1{}));
        sr.reset(res, make_const(test_source{}));
    }

    //--------------------------------------------

    void
    testEmptyBody()
    {
        auto const check =
        [](
            string_view headers,
            string_view expected)
        {
            auto res =
                make_response(headers);
            serializer sr;
            sr.reset(res);
            std::string s = read(sr);
            BOOST_TEST(s == expected);
        };

        check(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 0\r\n"
            "\r\n",
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 0\r\n"
            "\r\n");

        check(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n",
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "0\r\n\r\n");
    }

    //--------------------------------------------

    void
    check(
        string_view headers,
        string_view body,
        string_view expected)
    {
        auto res =
            make_response(headers);
        std::string sb = body;
        serializer sr;
        sr.reset(res,
            string_body(std::move(sb)));
        std::string s = read(sr);
        BOOST_TEST(s == expected);
    };

    template<class Source>
    void
    check_src(
        string_view headers,
        Source&& src,
        string_view expected)
    {
        auto res =
            make_response(headers);
        serializer sr;
        sr.reset(res, std::forward<
            Source>(src));
        std::string s = read(sr);
        BOOST_TEST(s == expected);
    };

    void
    testOutput()
    {
        // buffers (0 size)
        check(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 0\r\n"
            "\r\n",
            "",
            //--------------------------
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 0\r\n"
            "\r\n");

        // buffers
        check(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 5\r\n"
            "\r\n",
            "12345",
            //--------------------------
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "12345");

        // buffers chunked
        check(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n",
            "12345",
            //--------------------------
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "0000000000000005\r\n"
            "12345"
            "\r\n"
            "0\r\n"
            "\r\n");

        // source
        check_src(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 5\r\n"
            "\r\n",
            test_source{},
            //--------------------------
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "12345");

        // source chunked
        check_src(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n",
            test_source{},
            //--------------------------
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "0000000000000005\r\n"
            "12345"
            "\r\n"
            "0\r\n\r\n");
    }

    void
    testExpect100Continue()
    {
        // request
        {
            serializer sr;
            auto req = make_request(
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
            sr.reset(req, test_source{});
            std::string s;
            result<serializer::output_buffers> rv;
            for(;;)
            {
                rv = sr.prepare();
                if(! rv)
                    break;
                auto n = append(s, *rv);
                sr.consume(n);
            }
            BOOST_TEST(rv.error() ==
                error::expect_100_continue);
            BOOST_TEST(s ==
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
            while(! sr.is_done())
            {
                rv = sr.prepare();
                BOOST_TEST(! rv.has_error());
                auto n = append(s, *rv);
                sr.consume(n);
            }
            BOOST_TEST(s ==
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n"
                "12345");
        }

        // empty body
        {
            serializer sr;
            auto req = make_request(
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
            sr.reset(req);
            std::string s;
            result<serializer::output_buffers> rv;
            for(;;)
            {
                rv = sr.prepare();
                if(! rv)
                    break;
                auto n = append(s, *rv);
                sr.consume(n);
            }
            BOOST_TEST(rv.error() ==
                error::expect_100_continue);
            BOOST_TEST(s ==
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
            BOOST_TEST(! sr.is_done());
            rv = sr.prepare();
            BOOST_TEST(! rv.has_error());
            BOOST_TEST_EQ(buffer_size(*rv), 0);
            BOOST_TEST(! sr.is_done());
            sr.consume(0);
            BOOST_TEST(sr.is_done());
        }

        // response
        {
            string_view sv =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 5\r\n"
                "Expect: 100-continue\r\n"
                "\r\n";
            serializer sr;
            auto res = make_response(sv);
            sr.reset(res, test_source{});
            auto s = read(sr);
            BOOST_TEST(s ==
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 5\r\n"
                "Expect: 100-continue\r\n"
                "\r\n"
                "12345");
        }
    }

    void
    run()
    {
        testSyntax();
        testEmptyBody();
        testOutput();
        testExpect100Continue();
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost
