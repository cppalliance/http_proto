//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_parser.hpp>

#include <boost/http_proto/context.hpp>

#include "test_suite.hpp"

#include <algorithm>
#include <string>

namespace boost {
namespace http_proto {

class request_parser_test
{
public:
    context ctx_;

    bool
    valid(
        string_view s,
        std::size_t nmax)
    {
        request_parser p(ctx_);
        while(! s.empty())
        {
            auto b = p.prepare();
            auto n = b.second;
            if( n > s.size())
                n = s.size();
            if( n > nmax)
                n = nmax;
            std::memcpy(b.first,
                s.data(), n);
            p.commit(n);
            s.remove_prefix(n);
            error_code ec;
            p.parse_header(ec);
            if(ec == error::need_more)
                continue;
            return ! ec.failed();
        }
        return false;
    }

    void
    good(string_view s)
    {
        for(std::size_t nmax = 1;
            nmax < s.size(); ++nmax)
            BOOST_TEST(valid(s, nmax));
    }

    void
    bad(string_view s)
    {
        for(std::size_t nmax = 1;
            nmax < s.size(); ++nmax)
            BOOST_TEST(! valid(s, nmax));
    }

    void
    check(
        method m,
        string_view t,
        int v,
        string_view const s)
    {
        auto const f =
            [&](request_parser const& p)
        {
            auto const req = p.header();
            BOOST_TEST(req.method() == m);
            BOOST_TEST(req.method_str() ==
                to_string(m));
            BOOST_TEST(req.target() == t);
            BOOST_TEST(req.version() == v);
        };

        // single buffer
        {
            error_code ec;
            request_parser p(ctx_);
            auto const b = p.prepare();
            auto const n = (std::min)(
                b.second, s.size());
            BOOST_TEST(n == s.size());
            std::memcpy(
                b.first, s.data(), n);
            p.commit(n);
            p.parse_header(ec);
            BOOST_TEST(! ec);
            //BOOST_TEST(p.is_done());
            if(! ec)
                f(p);
        }

        // two buffers
        for(std::size_t i = 1;
            i < s.size(); ++i)
        {
            error_code ec;
            request_parser p(ctx_);
            // first buffer
            auto b = p.prepare();
            auto n = (std::min)(
                b.second, i);
            BOOST_TEST(n == i);
            std::memcpy(
                b.first, s.data(), n);
            p.commit(n);
            p.parse_header(ec);
            if(! BOOST_TEST(
                ec == error::need_more))
                continue;
            // second buffer
            b = p.prepare();
            n = (std::min)(
                b.second, s.size());
            BOOST_TEST(n == s.size());
            std::memcpy(
                b.first, s.data() + i, n);
            p.commit(n);
            p.parse_header(ec);
            BOOST_TEST(! ec);
            if(ec)
                continue;
            //BOOST_TEST(p.is_done());
            f(p);
        }
    }

    void
    testParse()
    {
        check(method::get, "/", 1,
            "GET / HTTP/1.1\r\n"
            "Connection: close\r\n"
            "\r\n");
    }

    void
    testParseField()
    {
        auto f = [](string_view f)
        {
            return std::string(
                "GET / HTTP/1.1\r\n") +
                std::string(
                    f.data(), f.size()) +
                "\r\n\r\n";
        };

        bad(f(":"));
        bad(f(" :"));
        bad(f(" x:"));
        bad(f("x :"));
        bad(f("x@"));
        bad(f("x@:"));
        bad(f("x: y \r\n \r\n"));

        good(f(""));
        good(f("x:"));
        good(f("x: "));
        good(f("x:\t "));
        good(f("x:y"));
        good(f("x: y"));
        good(f("x:y "));
        good(f("x: y "));
        good(f("x:yy"));
        good(f("x: yy"));
        good(f("x:yy "));
        good(f("x: y y "));
        good(f("x:"));
        good(f("x: \r\n "));
        good(f("x: \r\n x"));
        good(f("x: \r\n \t\r\n "));
        good(f("x: \r\n \t\r\n x"));
    }

    void
    run()
    {
//        testParse();
        testParseField();
    }
};

TEST_SUITE(request_parser_test, "boost.http_proto.request_parser");

} // http_proto
} // boost

