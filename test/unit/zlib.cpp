//
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/detail/config.hpp>

#include "test_suite.hpp"

#ifndef BOOST_HTTP_PROTO_HAS_ZLIB

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE("zlib not found, building dummy zlib.cpp test")

struct zlib_test
{
    void run()
    {}
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.zlib");

#else

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

#include <boost/buffers/algorithm.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/buffers/string_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <random>

#include <zlib.h>

// opt into random ascii generation as it's easier than
// maintaing a large static asset that has to be loaded
// at runtime
std::string
generate_book(std::size_t size)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> distrib(32, 127);

    std::string out(size, '\0');
    for(auto& o : out)
        o = static_cast<char>(distrib(gen));

    return out;
};

namespace boost {
namespace http_proto {

struct zlib_test
{
    struct faulty_zlib_service
        : public zlib::service
    {
        using key_type = service;

        struct faulty_stream : zlib::stream
        {
            system::error_code
            write(zlib::params&, zlib::flush) noexcept override
            {
                return zlib::error::version_err;
            }
        };

        explicit
        faulty_zlib_service(context&) noexcept
        {
        }

        std::size_t
        deflator_space_needed(
            int,
            int) const noexcept override
        {
            return 1;
        }

        std::size_t
        inflator_space_needed(
            int) const noexcept override
        {
            return 1;
        }

        zlib::stream&
        make_deflator(
            http_proto::detail::workspace& ws,
            int,
            int,
            int) const override
        {
            return ws.emplace<faulty_stream>();
        }

        zlib::stream&
        make_inflator(
            http_proto::detail::workspace& ws,
            int) const override
        {
            return ws.emplace<faulty_stream>();
        }
    };

    std::string
    deflate(
        int window_bits,
        int mem_level,
        core::string_view str)
    {
        ::z_stream zs{};

        if(!BOOST_TEST_EQ(
               deflateInit2(&zs, -1, Z_DEFLATED, window_bits,
               mem_level, Z_DEFAULT_STRATEGY), Z_OK))
        {
            ::deflateEnd(&zs);
            return {};
        }

        zs.next_in  = reinterpret_cast<Bytef*>(
            const_cast<char *>(str.data()));
        zs.avail_in = static_cast<uInt>(str.size());

        std::string result;
        for(;;)
        {
            constexpr auto chunk_size = 2048;
            result.resize(result.size() + chunk_size);
            auto it = result.begin() + result.size() - chunk_size;
            zs.next_out  = reinterpret_cast<Bytef*>(&*it);
            zs.avail_out = chunk_size;
            auto ret = ::deflate(&zs, Z_FINISH);
            result.erase(
                it + chunk_size - zs.avail_out, result.end());
            if( ret != Z_OK )
                break;
        }
        ::deflateEnd(&zs);
        return result;
    }

    void verify_compressed(
        span<unsigned char> compressed,
        core::string_view expected)
    {
        int ret = -1;

        ::z_stream zs{};

        int const window_bits = 15; // default
        int const enable_zlib_and_gzip = 32;
        ret = ::inflateInit2(
            &zs, window_bits + enable_zlib_and_gzip);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
        {
            ::inflateEnd(&zs);
            return;
        }

        std::vector<unsigned char> decompressed_output(
            2 * expected.size(), 0x00);

        zs.next_in = compressed.data();
        zs.avail_in =
            static_cast<unsigned>(compressed.size());

        zs.next_out = decompressed_output.data();
        zs.avail_out =
            static_cast<unsigned>(decompressed_output.size());

        ret = ::inflate(&zs, Z_FINISH);
        if(! BOOST_TEST_EQ(ret, Z_STREAM_END) )
        {
            ::inflateEnd(&zs);
            return;
        }

        auto n = zs.next_out - decompressed_output.data();
        core::string_view sv2(
            reinterpret_cast<char const*>(
                decompressed_output.data()), n);
        BOOST_TEST_EQ(sv2, expected);

        ::inflateEnd(&zs);
        return;
    }

    static
    buffers::mutable_buffer
    zlib_serializer_source(
        response_view res,
        serializer& sr,
        span<char const> body_view,
        span<unsigned char> output)
    {
        struct sink : public source
        {
            span<char const>& body_view_;
            sink(span<char const>& bv) : body_view_(bv) {}

            results
            on_read(buffers::mutable_buffer b)
            {
                results rs;
                auto n = buffers::buffer_copy(
                    b,
                    buffers::const_buffer(
                        body_view_.data(),
                        std::min(
                            std::size_t{512},
                            body_view_.size())));

                body_view_ = body_view_.subspan(n);
                rs.bytes = n;
                rs.ec = {};
                rs.finished = body_view_.empty();
                return rs;
            }
        };

        buffers::mutable_buffer output_buf(
            output.data(), output.size());

        auto& s = sr.start<sink>(res, body_view);
        (void)s;

        while(! body_view.empty() || ! sr.is_done() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::buffer_size(cbs), 0);

            auto n2 = buffers::buffer_copy(
                output_buf, cbs);
            BOOST_TEST_EQ(n2, buffers::buffer_size(cbs));

            sr.consume(n2);
            output_buf += n2;
        }
        return output_buf;
    }

    static
    buffers::mutable_buffer
    zlib_serializer_stream(
        response_view res,
        serializer& sr,
        span<char const> body_view,
        span<unsigned char> output)
    {
        buffers::mutable_buffer output_buf(
            output.data(), output.size());

        auto stream = sr.start_stream(res);

        while(! body_view.empty() )
        {
            auto mbs = stream.prepare();
            auto n = buffers::buffer_copy(
                mbs, buffers::const_buffer(
                    body_view.data(),
                    std::min(
                        std::size_t{512},
                        body_view.size())));

            BOOST_TEST_GT(n, 0);
            stream.commit(n);

            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::buffer_size(cbs), 0);

            auto n2 = buffers::buffer_copy(
                output_buf, cbs);
            BOOST_TEST_EQ(n2, buffers::buffer_size(cbs));
            sr.consume(n2);
            output_buf += n2;
            body_view = body_view.subspan(n);
        }
        stream.close();

        while(! sr.is_done() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::buffer_size(cbs), 0);
            auto n = buffers::buffer_copy(
                output_buf, cbs);
            output_buf += n;
            BOOST_TEST_EQ(n, buffers::buffer_size(cbs));
            sr.consume(n);
        }

        return output_buf;
    }

    static
    buffers::mutable_buffer
    zlib_serializer_buffers(
        response_view res,
        serializer& sr,
        span<char const> body_view,
        span<unsigned char> output)
    {
        buffers::mutable_buffer output_buf(
            output.data(), output.size());

        std::vector<buffers::const_buffer> buf_seq;
        {
            std::size_t num_bufs = 23; // random odd number
            std::size_t buf_size =
                body_view.size() / num_bufs;
            std::size_t remaining =
                body_view.size() % num_bufs;

            for( std::size_t i = 0; i < num_bufs; ++i )
            {
                auto offset = i * buf_size;
                buf_seq.push_back(
                   {body_view.data() + offset, buf_size});
            }

            if( remaining > 0 )
            {
                auto offset = num_bufs * buf_size;
                buf_seq.push_back(
                    {body_view.data() + offset, remaining});
            }

            for( auto buf : buf_seq )
                BOOST_TEST_GT(buf.size(), 0);
        }

        buffers::const_buffer_span bufs(
            buf_seq.data(), buf_seq.size());

        sr.start(res, bufs);

        while(! sr.is_done() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::buffer_size(cbs), 0);
            BOOST_TEST_LT(
                    buffers::buffer_size(cbs),
                    output_buf.size());

            auto n = buffers::buffer_copy(output_buf, cbs);
            BOOST_TEST_EQ(n, buffers::buffer_size(cbs));

            output_buf += n;
            sr.consume(n);
        }
        return output_buf;
    }

    using fp_type =
        buffers::mutable_buffer(*)(
            response_view res,
            serializer& sr,
            span<char const> body_view,
            span<unsigned char> output);

    void zlib_serializer_impl(
        fp_type fp,
        core::string_view const c,
        std::string const& body,
        bool chunked_encoding)
    {
        context ctx;
        zlib::install_service(ctx);
        serializer sr(
            ctx,
            ctx.get_service<
                zlib::service>().deflator_space_needed(15, 8) + (2 * 1024));

        // prove we can reuse the serializer successfully
        for( int i = 0; i < 2; ++i )
        {
            sr.reset();

            response res;
            res.set("Transfer-Encoding", c);
            res.set_chunked(chunked_encoding);

            std::string header;
            {
                header += "HTTP/1.1 200 OK\r\n";
                if( c == "deflate" )
                {
                    header += "Transfer-Encoding: deflate\r\n";
                    sr.use_deflate_encoding();
                }
                if( c == "gzip" )
                {
                    header += "Transfer-Encoding: gzip\r\n";
                    sr.use_gzip_encoding();
                }

                if( chunked_encoding )
                    header += "Transfer-Encoding: chunked\r\n";

                header += "\r\n";
            }

            core::string_view str = header;
            std::vector<unsigned char> output(
                str.size() + 3 * body.size(), 0x00);

            span<char const> body_view = body;
            auto output_buf =
                fp(res, sr, body_view, output);

            auto m = output.size() - output_buf.size();
            auto sv =
                core::string_view(
                        reinterpret_cast<char const*>(
                            output.data()),
                        m);

            BOOST_TEST(sv.starts_with(str));

            sv = sv.substr(str.size());

            std::vector<unsigned char> compressed;
            compressed.reserve(body_view.size());

            while(! sv.empty() )
            {
                if(! res.chunked() )
                {
                    compressed.insert(
                        compressed.end(), sv.begin(), sv.end());
                    break;
                }

                core::string_view& chunk = sv;

                auto pos = chunk.find_first_of("\r\n");
                BOOST_TEST_NE(pos, core::string_view::npos);

                std::string chunk_header = chunk.substr(0, pos);
                chunk.remove_prefix(pos + 2);

                auto chunk_size = std::stoul(
                    chunk_header, nullptr, 16);

                if( chunk_size == 0 )
                {
                    BOOST_TEST_EQ(chunk, "\r\n");
                }
                else
                {
                    BOOST_TEST_LT(
                        chunk.begin() + chunk_size,
                        chunk.end());

                    compressed.insert(
                        compressed.end(),
                        chunk.data(),
                        chunk.data() + chunk_size);

                    chunk.remove_prefix(chunk_size);
                    BOOST_TEST(chunk.starts_with("\r\n"));
                }
                chunk.remove_prefix(2);
            }

            // BOOST_TEST_LT(compressed.size(), body.size());

            verify_compressed(compressed, body);
        }
    }

    void
    test_serializer()
    {
        std::string short_body =
            "hello world, compression seems super duper cool! hmm, but what if I also add like a whole bunch of text to this thing????";

        std::string long_body =
            generate_book(350000);

        std::vector<core::string_view> bodies = {
            short_body,
            long_body
        };

        std::vector<core::string_view> coding_types = {
            "deflate",
            "gzip"
        };

        std::vector<fp_type> fps = {
            zlib_serializer_stream,
            zlib_serializer_source,
            zlib_serializer_buffers
        };

        bool use_chunked_encoding[] = {
            false,
            true
        };

        for( auto fp : fps )
        for( auto body : bodies )
        for( auto c : coding_types )
        for( auto b : use_chunked_encoding )
            zlib_serializer_impl(fp, c, body, b);
    }

    void
    test_parser()
    {
        context ctx;
        zlib::install_service(ctx);

        auto append_chunked = [](
            std::string& msg,
            core::string_view body)
        {
            for(;;)
            {
                auto chunk = body.substr(0,
                    std::min(size_t{ 100 }, body.size()));
                body.remove_prefix(chunk.size());
                msg.append(8, '0');
                auto it = msg.begin() + msg.size() - 8;
                auto c = std::snprintf(&*it, 8, "%zx", chunk.size());
                msg.erase(it + c, msg.end());
                msg += "\r\n";
                msg += chunk;
                msg += "\r\n";
                if(chunk.size() == 0)
                    break;
            }
        };

        response_parser::config cfg;
        cfg.apply_deflate_decoder = true;
        cfg.apply_gzip_decoder = true;
        cfg.body_limit = 8 * 1024 * 1024;
        install_parser_service(ctx, cfg);
        response_parser pr(ctx);

        for(auto gzip : { false, true })
        for(auto chunked : { false, true })
        for(auto body_size : { 0, 7, 64 * 1024, 1024 * 1024 })
        {
            std::string msg = "HTTP/1.1 200 OK\r\n";

            if(gzip)
                msg += "Content-Encoding: gzip\r\n";
            else
                msg += "Content-Encoding: deflate\r\n";

            if(chunked)
                msg += "Transfer-Encoding: chunked\r\n";

            msg += "\r\n";

            auto const body = generate_book(body_size);
            auto const deflated = deflate(15 + (gzip ? 16 : 0), 8, body);

            if(chunked)
                append_chunked(msg, deflated);
            else
                msg += deflated;

            pr.reset();
            pr.start();

            auto msg_buf =
                buffers::const_buffer{ msg.data(), msg.size() };
            
            std::string parsed_body;
            buffers::string_buffer parsed_body_buf{
                &parsed_body };
            for(;;)
            {
                auto n1 = buffers::buffer_copy(
                    pr.prepare(), msg_buf);
                pr.commit(n1);
                msg_buf = buffers::sans_prefix(msg_buf, n1);

                boost::system::error_code ec;
                pr.parse(ec);
                if( ec )
                    BOOST_TEST(ec == error::in_place_overflow
                        || ec == error::need_data);

                // consume in_place body
                auto n2 = buffers::buffer_copy(
                    parsed_body_buf.prepare(
                        buffers::buffer_size(pr.pull_body())),
                    pr.pull_body());
                parsed_body_buf.commit(n2);
                pr.consume_body(n2);

                if( msg_buf.size() == 0 && ec == error::need_data )
                {
                    pr.commit_eof();
                    pr.parse(ec);
                    BOOST_TEST(!ec || ec == error::in_place_overflow);
                }
                if( pr.is_complete() )
                    break;
            }
            BOOST_TEST(parsed_body == body);
        }
    }

    void
    test_serializer_reports_zlib_errors()
    {
        context ctx;
        ctx.make_service<faulty_zlib_service>();
        serializer sr{ ctx };

        response res{
            "HTTP/1.1 200 OK\r\n"
            "\r\n" };
        std::string buf(1024, '*');
        sr.use_gzip_encoding();
        sr.start(res, buffers::const_buffer{ buf.data(), buf.size() });

        auto rs = sr.prepare();
        BOOST_TEST(rs.has_error());
        BOOST_TEST_EQ(rs.error(), zlib::error::version_err);
    }

    void
    test_parser_reports_zlib_errors()
    {
        context ctx;
        ctx.make_service<faulty_zlib_service>();

        response_parser::config cfg;
        cfg.apply_deflate_decoder = true;
        install_parser_service(ctx, cfg);

        response_parser pr{ ctx };
        pr.reset();
        pr.start();

        std::string msg{
            "HTTP/1.1 200 OK\r\n"
            "Content-Encoding: deflate\r\n"
            "\r\n" };
        auto n = buffers::buffer_copy(
            pr.prepare(),
            buffers::const_buffer{ msg.data(), msg.size() });

        BOOST_TEST_EQ(n, msg.size());
        pr.commit(n);

        boost::system::error_code ec;
        pr.parse(ec);
        BOOST_TEST_EQ(ec, zlib::error::version_err);
    }

    void run()
    {
        test_serializer();
        test_serializer_reports_zlib_errors();
        test_parser();
        test_parser_reports_zlib_errors();
    }
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.zlib");

} // namespace http_proto
} // namespace boost

#endif
