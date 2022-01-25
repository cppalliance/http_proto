//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_parser.hpp>

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/version.hpp>

#include "test_suite.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace boost {
namespace http_proto {

class request_parser_test
{
public:
    context ctx_;

    bool
    feed(
        parser& p,
        string_view s)
    {
        while(! s.empty())
        {
            auto b = p.prepare();
            auto n = b.size();
            if( n > s.size())
                n = s.size();
            std::memcpy(b.data(),
                s.data(), n);
            p.commit(n);
            s.remove_prefix(n);
            error_code ec;
            p.parse_header(ec);
            if(ec == error::end_of_message
                || ! ec)
                break;
            if(! BOOST_TEST(
                ec == grammar::error::incomplete))
            {
                test_suite::debug_stream dout(
                    std::cout);
                dout << ec.message() << "\n";
                return false;
            }
        }
        return true;
    }

    bool
    valid(
        string_view s,
        std::size_t nmax)
    {
        request_parser p(ctx_);
        while(! s.empty())
        {
            auto b = p.prepare();
            auto n = b.size();
            if( n > s.size())
                n = s.size();
            if( n > nmax)
                n = nmax;
            std::memcpy(b.data(),
                s.data(), n);
            p.commit(n);
            s.remove_prefix(n);
            error_code ec;
            p.parse_header(ec);
            if(ec == grammar::error::incomplete)
                continue;
            auto const es = ec.message();
            return ec ==
                error::end_of_message;
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
        version v,
        string_view const s)
    {
        auto const f =
            [&](request_parser const& p)
        {
            auto const req = p.get();
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
                b.size(), s.size());
            BOOST_TEST(n == s.size());
            std::memcpy(
                b.data(), s.data(), n);
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
                b.size(), i);
            BOOST_TEST(n == i);
            std::memcpy(
                b.data(), s.data(), n);
            p.commit(n);
            p.parse_header(ec);
            if(! BOOST_TEST(
                ec == grammar::error::incomplete))
                continue;
            // second buffer
            b = p.prepare();
            n = (std::min)(
                b.size(), s.size());
            BOOST_TEST(n == s.size());
            std::memcpy(
                b.data(), s.data() + i, n - i);
            p.commit(n);
            p.parse_header(ec);
            BOOST_TEST(! ec ||
                ec == error::end_of_message);
            if(ec.failed())
                continue;
            //BOOST_TEST(p.is_done());
            f(p);
        }
    }

    void
    testParse()
    {
        check(method::get, "/",
            version::http_1_1,
            "GET / HTTP/1.1\r\n"
            "Connection: close\r\n"
            "Content-Length: 42\r\n"
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
        good(f("x: y \r\n "));

        // errata eid4189
        good(f("x: , , ,"));
        good(f("x: abrowser/0.001 (C O M M E N T)"));
        good(f("x: gzip , chunked"));
    }

    void
    testGet()
    {
        request_parser p(ctx_);
        string_view s = 
            "GET / HTTP/1.1\r\n"
            "User-Agent: x\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "a: 1\r\n"
            "b: 2\r\n"
            "a: 3\r\n"
            "c: 4\r\n"
            "\r\n";

        feed(p, s);

        auto const rv = p.get();
        BOOST_TEST(
            rv.method() == method::get);
        BOOST_TEST(
            rv.method_str() == "GET");
        BOOST_TEST(rv.target() == "/");
        BOOST_TEST(rv.version() ==
            version::http_1_1);
        auto const h = rv.fields;
        //User-Agent: xrnConnection: closernTransfer-Encoding: chunkedrna: 1rnb: 2rna: 3rnc: 4rnrn

        BOOST_TEST(rv.get_const_buffer() == s);
        BOOST_TEST(h.get_const_buffer() ==
            "User-Agent: x\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n"
            "a: 1\r\n"
            "b: 2\r\n"
            "a: 3\r\n"
            "c: 4\r\n"
            "\r\n");
        BOOST_TEST(h.size() == 7);
        BOOST_TEST(h[0].value == "x");
        BOOST_TEST(
            h.exists(field::connection));
        BOOST_TEST(! h.exists(field::age));
        BOOST_TEST(h.exists("Connection"));
        BOOST_TEST(h.exists("CONNECTION"));
        BOOST_TEST(! h.exists("connector"));
        BOOST_TEST(h.count(
            field::transfer_encoding) == 1);
        BOOST_TEST(
            h.count(field::age) == 0);
        BOOST_TEST(
            h.count("connection") == 1);
        BOOST_TEST(h.count("a") == 2);
        BOOST_TEST_NO_THROW(
            h.at(field::user_agent) == "x");
        BOOST_TEST_NO_THROW(
            h.at("a") == "1");
        BOOST_TEST_THROWS(h.at(field::age),
            std::exception);
        BOOST_TEST_THROWS(h.at("d"),
            std::exception);
        BOOST_TEST(
            h.value_or("a", "x") == "1");
        BOOST_TEST(
            h.value_or("d", "x") == "x");
        BOOST_TEST(h.value_or(
            field::age, "x") == "x");
        BOOST_TEST(h.value_or(
            field::user_agent, {}) == "x");
        BOOST_TEST(h.find(
            field::connection)->id ==
                field::connection);
        BOOST_TEST(
            h.find("a")->value == "1");
        BOOST_TEST(h.matching(
            field::user_agent).make_list() == "x");
        BOOST_TEST(h.matching(
            "b").make_list() == "2");
        BOOST_TEST(h.matching(
            "a").make_list() == "1,3");
    }

    void
    run()
    {
        BOOST_TEST(valid(
            "GET / HTTP/1.1\r\n"
            "x:\r\n"
            "\r\n", 1));

        testParse();
        testParseField();
        testGet();
    }
};

TEST_SUITE(request_parser_test, "boost.http_proto.request_parser");

} // http_proto
} // boost


