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
#include <algorithm>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

namespace net {
namespace error {
static int eof = 1;
} // error
} // net

class request_parser_test
{
public:
    context ctx_;

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

    struct socket
    {
    };

    template<
        class ReadStream>
    void
    read_some(
        ReadStream&,
        basic_parser&,
        error_code&)
    {
    }

    template<
        class ReadStream>
    void
    read_header(
        ReadStream& sock,
        basic_parser& p,
        error_code& ec)
    {
        for(;;)
        {
            p.parse_header(ec);
            if(ec != error::need_more)
                break;
            read_some(sock, p, ec);
            if(ec == error::eof)
                p.commit_eof();
            else if(ec.failed())
                return;
        }
    }

    template<
        class ReadStream>
    void
    read_body(
        ReadStream& sock,
        basic_parser& p,
        error_code& ec)
    {
        for(;;)
        {
            p.parse_body(ec);
            if(ec != error::need_more)
                break;
            read_some(sock, p, ec);
            if(ec == error::eof)
                p.commit_eof();
            else if(ec.failed())
                return;
        }
    }

    template<
        class ReadStream>
    void
    read_body_part(
        ReadStream& sock,
        basic_parser& p,
        error_code& ec)
    {
        for(;;)
        {
            p.parse_body_part(ec);
            if(ec != error::need_more)
                break;
            read_some(sock, p, ec);
            if(ec == error::eof)
                p.commit_eof();
            else if(ec.failed())
                return;
        }
    }

    template<
        class ReadStream>
    void
    read_chunk_ext(
        ReadStream& sock,
        basic_parser& p,
        error_code& ec)
    {
        for(;;)
        {
            p.parse_chunk_ext(ec);
            if(ec != error::need_more)
                break;
            read_some(sock, p, ec);
            if(ec == error::eof)
                p.commit_eof();
            else if(ec.failed())
                return;
        }
    }

    template<
        class ReadStream>
    void
    read_chunk_part(
        ReadStream& sock,
        basic_parser& p,
        error_code& ec)
    {
        for(;;)
        {
            p.parse_chunk_part(ec);
            if(ec != error::need_more)
                break;
            read_some(sock, p, ec);
            if(ec == error::eof)
                p.commit_eof();
            else if(ec.failed())
                return;
        }
    }

    template<
        class ReadStream>
    void
    read_chunk_trailer(
        ReadStream& sock,
        basic_parser& p,
        error_code& ec)
    {
        for(;;)
        {
            p.parse_chunk_trailer(ec);
            if(ec != error::need_more)
                break;
            read_some(sock, p, ec);
            if(ec == error::eof)
                p.commit_eof();
            else if(ec.failed())
                return;
        }
    }

    //
    // Read complete body into parser-owned storage
    //
    void
    doReadBody()
    {
        socket sock;
        error_code ec;
        request_parser p(ctx_);

        read_header(sock, p, ec);
        if(ec)
            return;
        request_view req = p.header();

        read_body(sock, p, ec);
        if(ec)
            return;
        // access complete body
        string_view body = p.body();
    }

    //
    // Read a complete body a buffer at a time,
    // using parser-owned storage.
    //
    void
    doReadBodyParts()
    {
        socket sock;
        error_code ec;
        request_parser p(ctx_);

        read_header(sock, p, ec);
        if(ec)
            return;
        request_view req = p.header();

        for(;;)
        {
            read_body_part(sock, p, ec);
            if(ec == error::end_of_body)
                break;
            if(ec.failed())
                return;
            // access body part
            string_view body_part = p.body();
        }
    }

    //
    // Read a chunked body one chunk
    // at a time, including extensions
    // and the trailer
    //
    void
    doReadChunked()
    {
        socket sock;
        error_code ec;
        request_parser p(ctx_);

        read_header(sock, p, ec);
        if(ec)
            return;
        request_view req = p.header();

        for(;;)
        {
            read_chunk_part(sock, p, ec);
            if(ec == error::end_of_body)
                break;
            if(ec.failed())
                return;
            // access body part
            string_view body_part = p.body();
        }
    }

    void
    run()
    {
#if 0
        doReadBody();
        doReadBodyParts();
#endif
        testParse();
    }
};

TEST_SUITE(request_parser_test, "boost.http_proto.request_parser");

} // http_proto
} // boost

