//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/serializer.hpp>

#include <boost/http_proto/response.hpp>
#include <boost/http_proto/string_body.hpp>
#include <boost/buffers/algorithm.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/buffers/mutable_buffer.hpp>
#include <boost/core/ignore_unused.hpp>
#include "test_helpers.hpp"

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef BOOST_HTTP_PROTO_HAS_ZLIB
#include <zlib.h>
#endif

namespace boost {
namespace http_proto {

struct serializer_test
{
    struct test_source : source
    {
        test_source(core::string_view s)
            : s_(s)
        {
        }

        results
        on_read(
            buffers::mutable_buffer b) override
        {
            BOOST_TEST(! is_done_);
            results rv;
            rv.bytes =
                buffers::buffer_copy(
                    b,
                    buffers::make_buffer(
                        s_.data(),
                        s_.size()));
            s_ = s_.substr(rv.bytes);
            rv.finished = s_.empty();
            is_done_ = rv.finished;
            return rv;
        }

    private:
        core::string_view s_;
        bool is_done_ = false;
    };

    struct faulty_source : source
    {
        faulty_source(system::error_code ec)
            : ec_{ ec }
        {
        }

        results
        on_read(buffers::mutable_buffer) override
        {
            BOOST_TEST(!is_done_);
            is_done_ = true;
            results rv;
            rv.ec = ec_;
            return rv;
        }

    private:
        system::error_code ec_;
        bool is_done_ = false;
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
        auto n = buffers::buffer_size(src);
        dest.resize(n0 + n);
        buffers::buffer_copy(
            buffers::mutable_buffer(
                &dest[n0], n), src);
        return n;
    }

    static
    std::string
    read_some(serializer& sr)
    {
        auto cbs = sr.prepare().value();
        BOOST_TEST(!sr.is_done());
        // We limit buffer consumption to necessitate
        // multiple calls to serializer::prepare() and
        // serializer::consume(), allowing tests to cover
        // state management within these functions
        std::string s;
        for( auto buf : buffers::prefix(cbs, 256) )
        {
            s.append(
                reinterpret_cast<char const*>(buf.data()),
                buf.size());
            sr.consume(buf.size());
        }
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

    static
    void
    check_chunked_body(
        core::string_view chunked_body,
        core::string_view expected_contents)
    {
        for(;;)
        {
            auto n = chunked_body.find_first_of("\r\n");
            BOOST_TEST_NE(n, core::string_view::npos);
            std::string tmp = chunked_body.substr(0, n);
            chunked_body.remove_prefix(n + 2);
            auto chunk_size = std::stoul(tmp, nullptr, 16);

            if( chunk_size == 0 ) // last chunk
            {
                BOOST_TEST_EQ(chunked_body, "\r\n");
                BOOST_TEST_EQ(chunked_body.size(), 2);
                chunked_body.remove_prefix(2);
                break;
            }

            BOOST_TEST_GE(expected_contents.size(), chunk_size);
            BOOST_TEST(chunked_body.starts_with(
                expected_contents.substr(0, chunk_size)));
            chunked_body.remove_prefix(chunk_size);
            expected_contents.remove_prefix(chunk_size);

            BOOST_TEST(chunked_body.starts_with("\r\n"));
            chunked_body.remove_prefix(2);
        }
        BOOST_TEST(chunked_body.empty());
        BOOST_TEST(expected_contents.empty());
    }

    //--------------------------------------------

    void
    testSyntax()
    {
        context ctx;
        serializer sr(ctx, 1024);
        response res;

        sr.start(res);
        sr.reset();

        sr.start(res, buffers::const_buffer{});
        sr.reset();

        sr.start(res, buffers::mutable_buffer{});
        sr.reset();

        sr.start<test_source>(res, "12345");
        sr.reset();

        sr.start(res, buffers::const_buffer{});
        sr.reset();

        sr.start(res, buffers::mutable_buffer{});
        sr.reset();

        sr.start<test_source>(res, "12345");
        sr.reset();

        sr.start_stream(res);
        sr.reset();

        serializer(ctx, 65536);
#ifdef BOOST_HTTP_PROTO_HAS_ZLIB
#if 0
        serializer(65536, gzip_decoder);
        serializer(65536, gzip_encoder);
        serializer(65536, gzip_decoder, gzip_encoder);
#endif
#endif
    }

    //--------------------------------------------

    void
    testEmptyBody()
    {
        auto const check =
        [](
            core::string_view headers,
            core::string_view expected)
        {
            response res(headers);
            context ctx;
            serializer sr(ctx);
            sr.start(res);
            std::string s = read(sr);
            BOOST_TEST_EQ(s, expected);
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
        core::string_view headers,
        core::string_view body,
        core::string_view expected)
    {
        response res(headers);
        std::string sb = body;

        context ctx;
        serializer sr(ctx);
        sr.start(res,
            string_body(std::move(sb)));
        std::string s = read(sr);
        BOOST_TEST_EQ(s, expected);
    };

    template<class Source, class F>
    void
    check_src(
        core::string_view headers,
        Source&& src,
        F const& f)
    {
        response res(headers);
        // we limit the buffer size of the serializer, requiring
        // it to make multiple calls to source::read
        context ctx;
        serializer sr(ctx, 1024);
        sr.start<Source>(res, std::forward<
            Source>(src));
        std::string s = read(sr);
        f(s);
    }

    struct check_stream_opts
    {
        std::size_t sr_capacity = 1024;
    };

    template <class F>
    void
    check_stream(
        core::string_view headers,
        core::string_view body,
        check_stream_opts const& opts,
        F f)
    {
        auto sr_capacity = opts.sr_capacity;

        response res(headers);

        context ctx;
        serializer sr(ctx, sr_capacity);
        auto stream = sr.start_stream(res);
        BOOST_TEST_EQ(stream.size(), 0);
        BOOST_TEST_GT(stream.capacity(), 0);
        BOOST_TEST_LE(stream.capacity(), sr_capacity);

        auto const N = stream.size() + stream.capacity();
        auto check_N = [&]
        {
            BOOST_TEST_EQ(
                stream.capacity() + stream.size(), N);
        };

        std::vector<char> s; // stores complete output

        auto prepare_chunk = [&]
        {
            BOOST_TEST(!stream.is_full());
            auto mbs = stream.prepare();

            auto bs = buffers::buffer_size(mbs);
            BOOST_TEST_GT(bs, 0);

            if( bs > body.size() )
                bs = body.size();

            buffers::buffer_copy(
                mbs, buffers::const_buffer(body.data(), bs));

            stream.commit(bs);
            if( bs < body.size() )
                BOOST_TEST(stream.is_full());
            else
                BOOST_TEST(!stream.is_full());

            body.remove_prefix(bs);
            // if(! res.chunked() )
            //     BOOST_TEST_EQ(stream.size(), bs);
            // else
            //     // chunk overhead: header + \r\n
            //     BOOST_TEST_EQ(stream.size(), bs + 18 + 2);

            check_N();
        };

        auto consume_body_buffer = [&](
            buffers::const_buffer buf)
        {
            // we have the prepared buffer sequence
            // representing the serializer's output but we
            // wish to emulate a user consuming it using a
            // smaller, fixed-size buffer
            std::array<char, 16> storage{};
            buffers::mutable_buffer out_buf(
                storage.data(), storage.size());

            while( buf.size() > 0 )
            {
                auto num_copied =
                    buffers::buffer_copy(out_buf, buf);

                buf += num_copied;

                s.insert(
                    s.end(),
                    storage.begin(),
                    storage.begin() + num_copied);

                sr.consume(num_copied);
            }
        };

        bool closed = false;
        while(! sr.is_done() )
        {
            if(! body.empty() )
                prepare_chunk();

            if( body.empty() && !closed )
            {
                stream.close();
                closed = true;
            }

            auto mcbs = sr.prepare();
            BOOST_TEST(!mcbs.has_error());

            auto cbs = mcbs.value();
            auto end = cbs.end();
            auto size = buffers::buffer_size(cbs);
            BOOST_TEST_GT(size, 0);

            for(auto pos = cbs.begin(); pos != end; ++pos)
                consume_body_buffer(*pos);
            // BOOST_TEST_EQ(stream.size(), 0);
            check_N();
        }

        BOOST_TEST_THROWS(stream.close(), std::logic_error);

        f(core::string_view(s.data(), s.size()));
    }

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
            "Content-Length: 2048\r\n"
            "\r\n",
            test_source{std::string(2048, '*')},
            [](core::string_view s){
                core::string_view header =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Content-Length: 2048\r\n"
                    "\r\n";

                BOOST_TEST_EQ(
                    s.substr(0, header.size()), header);
                BOOST_TEST(s ==
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Content-Length: 2048\r\n"
                    "\r\n" +
                    std::string(2048, '*'));
            });

        // empty source
        check_src(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 0\r\n"
            "\r\n",
            test_source{""},
            [](core::string_view s){
                BOOST_TEST(s ==
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n");
            });

        // faulty source
        {
            response res(
                "HTTP/1.1 200 OK\r\n"
                "\r\n");
            context ctx;
            serializer sr(ctx);

            sr.start<faulty_source>(
                res, system::error_code(4224, system::system_category()));

            auto rs = sr.prepare();
            BOOST_TEST(rs.has_error());
            BOOST_TEST_EQ(rs.error().value(), 4224);
        }

        // source chunked
        check_src(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n",
            test_source{std::string(2048, '*')},
            [](core::string_view s){
                core::string_view expected_header =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Transfer-Encoding: chunked\r\n"
                    "\r\n";
                BOOST_TEST(s.starts_with(expected_header));
                s.remove_prefix(expected_header.size());
                check_chunked_body(s, std::string(2048, '*'));
            });

        // empty source chunked
        check_src(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n",
            test_source{""},
            [](core::string_view s){
                core::string_view expected_header =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Transfer-Encoding: chunked\r\n"
                    "\r\n";
                BOOST_TEST(s.starts_with(expected_header));
                s.remove_prefix(expected_header.size());
                check_chunked_body(s, "");
            });

        // empty stream
        {
            check_stream_opts opts;
            check_stream(
                "HTTP/1.1 200 OK\r\n"
                "Server: test\r\n"
                "\r\n",
                std::string(0, '*'),
                opts,
                [](core::string_view s)
                {
                    core::string_view expected_header =
                        "HTTP/1.1 200 OK\r\n"
                        "Server: test\r\n"
                        "\r\n";
                    BOOST_TEST(s.starts_with(expected_header));
                    s.remove_prefix(expected_header.size());
                    BOOST_TEST(s.empty());
                });
        }

        // empty stream, chunked
        {
            check_stream_opts opts;
            check_stream(
                "HTTP/1.1 200 OK\r\n"
                "Server: test\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n",
                std::string(0, '*'),
                opts,
                [](core::string_view s)
                {
                    core::string_view expected_header =
                        "HTTP/1.1 200 OK\r\n"
                        "Server: test\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "\r\n";
                    BOOST_TEST(s.starts_with(expected_header));
                    s.remove_prefix(expected_header.size());
                    check_chunked_body(s, "");
                });
        }

        // stream
        {
            check_stream_opts opts;
            check_stream(
                "HTTP/1.1 200 OK\r\n"
                "Server: test\r\n"
                "Content-Length: 13370\r\n"
                "\r\n",
                std::string(13370, '*'),
                opts,
                [](core::string_view s){
                    core::string_view expected_header =
                        "HTTP/1.1 200 OK\r\n"
                        "Server: test\r\n"
                        "Content-Length: 13370\r\n"
                        "\r\n";

                    BOOST_TEST(
                        s.starts_with(expected_header));

                    s.remove_prefix(expected_header.size());
                    BOOST_TEST_EQ(
                        s, std::string(13370, '*'));
                });
        }

        // stream, chunked
        {
            check_stream_opts opts;
            check_stream(
                "HTTP/1.1 200 OK\r\n"
                "Server: test\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n",
                std::string(13370, '*'),
                opts,
                [](core::string_view s)
                {
                    core::string_view expected_header =
                        "HTTP/1.1 200 OK\r\n"
                        "Server: test\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "\r\n";
                    BOOST_TEST(s.starts_with(
                        expected_header));
                    s.remove_prefix(expected_header.size());
                    check_chunked_body(
                        s, std::string(13370, '*'));
                });
        }
    }

    void
    testExpect100Continue()
    {
        // request
        {
            context ctx;
            serializer sr(ctx);
            request req(
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
            sr.start<test_source>(req, "12345");
            std::string s;
            system::result<
                serializer::const_buffers_type> rv;
            BOOST_TEST_THROWS(
                sr.consume(req.buffer().size() + 1),
                std::invalid_argument);
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
            BOOST_TEST_THROWS(
                sr.prepare(), std::logic_error);
            BOOST_TEST(s ==
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n"
                "12345");
        }

        // empty body
        {
            context ctx;
            serializer sr(ctx);
            request req(
                "GET / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
            sr.start(req);
            std::string s;
            system::result<
                serializer::const_buffers_type> rv;
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
            BOOST_TEST_EQ(
                buffers::buffer_size(*rv), 0);
            BOOST_TEST(! sr.is_done());
            sr.consume(0);
            BOOST_TEST(sr.is_done());
        }

        // response
        {
            core::string_view sv =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 5\r\n"
                "Expect: 100-continue\r\n"
                "\r\n";

            context ctx;
            serializer sr(ctx);
            response res(sv);
            sr.start<test_source>(res, "12345");
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
    testStreamErrors()
    {
        {
            core::string_view sv =
                "HTTP/1.1 200 OK\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n";

            core::string_view serialized =
                "HTTP/1.1 200 OK\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"
                "0\r\n\r\n";

            response res(sv);

            context ctx;
            serializer sr(ctx);
            auto stream = sr.start_stream(res);
            auto mbs = stream.prepare();
            BOOST_TEST_GT(
                buffers::buffer_size(mbs), 0);
            BOOST_TEST(!stream.is_full());
            BOOST_TEST_THROWS(
                stream.commit(0), std::logic_error);

            auto mcbs = sr.prepare();
            BOOST_TEST(mcbs.has_error());
            BOOST_TEST(mcbs.error() == error::need_data);

            stream.close();

            mcbs = sr.prepare();
            auto cbs = mcbs.value();
            BOOST_TEST_EQ(
                buffers::buffer_size(cbs),
                serialized.size());

            std::vector<char> s(
                buffers::buffer_size(cbs), 'a');
            buffers::buffer_copy(
                buffers::make_buffer(
                    s.data(), s.size()),
                cbs);

            core::string_view octets(s.data(),s.size());
            BOOST_TEST_EQ(
                octets, serialized);

            sr.consume(s.size());
            BOOST_TEST(sr.is_done());
            BOOST_TEST_THROWS(
                sr.consume(s.size()),
                std::logic_error);
        }
    }

    void
    run()
    {
        testSyntax();
        testEmptyBody();
        testOutput();
        testExpect100Continue();
        testStreamErrors();
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost
