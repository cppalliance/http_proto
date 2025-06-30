//
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/error.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/serializer.hpp>

#include <boost/buffers/copy.hpp>
#include <boost/buffers/size.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/buffers/string_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>
#include <boost/rts/zlib.hpp>

#include "test_helpers.hpp"

#include <string>
#include <vector>
#include <random>

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
    std::string
    deflate(
        rts::zlib::deflate_service& d_svc,
        int window_bits,
        int mem_level,
        core::string_view str)
    {
        namespace zlib = rts::zlib;
        zlib::stream zs{};

        auto ret = static_cast<zlib::error>(
            d_svc.init2(
                zs,
                zlib::default_compression,
                zlib::deflated,
                window_bits,
                mem_level,
                zlib::default_strategy));

        if(!BOOST_TEST_EQ(ret, zlib::error::ok))
        {
            d_svc.deflate_end(zs);
            return {};
        }

        zs.next_in  = reinterpret_cast<unsigned char*>(const_cast<char *>(str.data()));
        zs.avail_in = static_cast<unsigned int>(str.size());

        std::string result;
        for(;;)
        {
            constexpr auto chunk_size = 2048;
            result.resize(result.size() + chunk_size);
            auto it = result.begin() + result.size() - chunk_size;
            zs.next_out  = reinterpret_cast<unsigned char*>(&*it);
            zs.avail_out = chunk_size;
            ret = static_cast<zlib::error>(
                d_svc.deflate(zs, zlib::finish));
            result.erase(
                it + chunk_size - zs.avail_out, result.end());
            if(ret != zlib::error::ok)
                break;
        }
        d_svc.deflate_end(zs);
        return result;
    }

    void verify_compressed(
        rts::zlib::inflate_service& i_svc,
        span<unsigned char> compressed,
        core::string_view expected)
    {
        namespace zlib = rts::zlib;

        zlib::stream zs{};

        int const window_bits = 15; // default
        int const enable_zlib_and_gzip = 32;
        auto ret = static_cast<zlib::error>(
            i_svc.init2(
                zs,
                window_bits + enable_zlib_and_gzip));
        if(! BOOST_TEST_EQ(ret, zlib::error::ok) )
        {
            i_svc.inflate_end(zs);
            return;
        }

        std::vector<unsigned char> decompressed_output(
            2 * expected.size() + 50, 0x00);

        zs.next_in = compressed.data();
        zs.avail_in =
            static_cast<unsigned>(compressed.size());

        zs.next_out = decompressed_output.data();
        zs.avail_out =
            static_cast<unsigned>(decompressed_output.size());

        ret = static_cast<zlib::error>(i_svc.inflate(zs, zlib::finish));
        if(! BOOST_TEST_EQ(ret, zlib::error::stream_end) )
        {
            i_svc.inflate_end(zs);
            return;
        }

        auto n = zs.next_out - decompressed_output.data();
        core::string_view sv2(
            reinterpret_cast<char const*>(
                decompressed_output.data()), n);
        BOOST_TEST(sv2 == expected);

        i_svc.inflate_end(zs);
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
        struct source_t : public source
        {
            span<char const>& body_view_;
            source_t(span<char const>& bv) : body_view_(bv) {}

            results
            on_read(buffers::mutable_buffer b)
            {
                results rs;
                auto n = buffers::copy(
                    b,
                    buffers::const_buffer(
                        body_view_.data(),
                        body_view_.size()));

                body_view_ = body_view_.subspan(n);
                rs.bytes = n;
                rs.ec = {};
                rs.finished = body_view_.empty();
                return rs;
            }
        };

        buffers::mutable_buffer output_buf(
            output.data(), output.size());

        auto& s = sr.start<source_t>(res, body_view);
        (void)s;

        while(! body_view.empty() || ! sr.is_done() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::size(cbs), 0);

            auto n2 = buffers::copy(
                output_buf, cbs);
            BOOST_TEST_EQ(n2, buffers::size(cbs));

            sr.consume(n2);
            output_buf = buffers::sans_prefix(output_buf, n2);
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
            auto n = buffers::copy(
                mbs, buffers::const_buffer(
                    body_view.data(),
                    std::min(
                        std::size_t{512},
                        body_view.size())));
            body_view = body_view.subspan(n);
            stream.commit(n);                

            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::size(cbs), 0);

            auto n2 = buffers::copy(
                output_buf, cbs);
            BOOST_TEST_EQ(n2, buffers::size(cbs));
            sr.consume(n2);
            output_buf = buffers::sans_prefix(output_buf, n2);
        }

        stream.close();

        while(! sr.is_done() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::size(cbs), 0);
            auto n = buffers::copy(
                output_buf, cbs);
            output_buf = buffers::sans_prefix(output_buf, n);
            BOOST_TEST_EQ(n, buffers::size(cbs));
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
        }

        buffers::const_buffer_span bufs(
            buf_seq.data(), buf_seq.size());

        sr.start(res, bufs);

        while(! sr.is_done() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::size(cbs), 0);
            BOOST_TEST_LT(
                    buffers::size(cbs),
                    output_buf.size());

            auto n = buffers::copy(output_buf, cbs);
            BOOST_TEST_EQ(n, buffers::size(cbs));

            output_buf = buffers::sans_prefix(output_buf, n);
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
        namespace zlib = rts::zlib;
        rts::context ctx;
        zlib::install_deflate_service(ctx);
        auto& i_svc = zlib::install_inflate_service(ctx);
        serializer::config cfg;
        cfg.apply_deflate_encoder = true;
        cfg.apply_gzip_encoder = true;
        install_serializer_service(ctx, cfg);
        serializer sr(ctx);

        // prove we can reuse the serializer successfully
        for( int i = 0; i < 2; ++i )
        {
            sr.reset();

            response res;
            res.set("Content-Encoding", c);
            res.set_chunked(chunked_encoding);

            std::string header;
            {
                header += "HTTP/1.1 200 OK\r\n";
                if( c == "deflate" )
                    header += "Content-Encoding: deflate\r\n";
                if( c == "gzip" )
                    header += "Content-Encoding: gzip\r\n";
                if( chunked_encoding )
                    header += "Transfer-Encoding: chunked\r\n";

                header += "\r\n";
            }

            core::string_view str = header;
            std::vector<unsigned char> output(
                str.size() + 3 * body.size() + 50, 0x00);

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
                    BOOST_TEST(chunk == "\r\n");
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

            verify_compressed(i_svc, compressed, body);
        }
    }

    void
    test_serializer()
    {
        std::string empty_body = "";

        std::string short_body =
            "hello world, compression seems super duper cool! hmm, but what if I also add like a whole bunch of text to this thing????";

        std::string long_body =
            generate_book(350000);

        std::vector<core::string_view> bodies = {
            empty_body,
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

    static
    std::string
    pull_body(
        response_parser& pr,
        buffers::const_buffer input)
    {
        std::string rs;
        buffers::string_buffer buf(&rs);
        for(;;)
        {
            auto n1 = buffers::copy(
                pr.prepare(), input);
            pr.commit(n1);
            input = buffers::sans_prefix(input, n1);

            boost::system::error_code ec;
            pr.parse(ec);
            if(!ec)
                pr.parse(ec);
            if(ec)
                BOOST_TEST(ec == error::in_place_overflow
                    || ec == error::need_data);

            // consume in_place body
            auto n2 = buffers::copy(
                buf.prepare(buffers::size(pr.pull_body())),
                pr.pull_body());
            buf.commit(n2);
            pr.consume_body(n2);

            if( input.size() == 0 && ec == error::need_data )
            {
                pr.commit_eof();
                pr.parse(ec);
                BOOST_TEST(!ec || ec == error::in_place_overflow);
            }
            if( pr.is_complete() )
                break;
        }
        return rs;
    }

    static
    std::string
    elastic_body(
        response_parser& pr,
        buffers::const_buffer input)
    {
        std::string rs;
        std::size_t n1 = buffers::copy(
                pr.prepare(), input);
        input = buffers::sans_prefix(input, n1);
        pr.commit(n1);
        system::error_code ec;
        pr.parse(ec);
        BOOST_TEST(pr.got_header());

        buffers::string_buffer buf(&rs);
        pr.set_body(std::ref(buf));
        pr.parse(ec);

        while(ec == error::need_data)
        {
            std::size_t n2 = buffers::copy(
                    pr.prepare(), input);
            input = buffers::sans_prefix(input, n2);
            pr.commit(n2);
            pr.parse(ec);
            if(n2 == 0)
            {
                pr.commit_eof();
                pr.parse(ec);
                break;
            }
        }
        return rs;
    }

    static
    std::string
    sink_body(
        response_parser& pr,
        buffers::const_buffer input)
    {
        std::size_t n1 = buffers::copy(
                pr.prepare(), input);
        input = buffers::sans_prefix(input, n1);
        pr.commit(n1);
        system::error_code ec;
        pr.parse(ec);
        BOOST_TEST(pr.got_header());

        class sink_t : public sink
        {
            std::string body_;

        public:
            std::string
            get_body()
            {
                return body_;
            }

            results
            on_write(
                buffers::const_buffer b,
                bool) override
            {
                body_.append(
                    static_cast<const char*>(b.data()),
                    b.size());
                results rv;
                rv.bytes = b.size();
                return rv;
            }
        };

        auto& sink = pr.set_body<sink_t>();
        pr.parse(ec);

        while(ec == error::need_data)
        {
            std::size_t n2 = buffers::copy(
                    pr.prepare(), input);
            input = buffers::sans_prefix(input, n2);
            pr.commit(n2);
            pr.parse(ec);
            if(n2 == 0)
            {
                pr.commit_eof();
                pr.parse(ec);
                break;
            }
        }
        return sink.get_body();
    }

    void
    test_parser()
    {
        rts::context ctx;
        rts::zlib::install_inflate_service(ctx);
        auto& d_svc = rts::zlib::install_deflate_service(ctx);

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
        cfg.body_limit = 1024 * 1024;
        install_parser_service(ctx, cfg);
        response_parser pr(ctx);
        pr.reset();

        for(std::string encoding : { "gzip", "deflate" })
        for(std::string transfer : { "chunked", "sized", "to_eof" })
        for(auto body_size : { 0, 7, 64 * 1024, 1024 * 1024 })
        for(auto receiver : { pull_body, sink_body, elastic_body })
        {
            std::string msg =
                "HTTP/1.1 200 OK\r\n"
                "Content-Encoding: " + encoding + "\r\n";

            auto body = generate_book(body_size);
            auto deflated_body = deflate(
                d_svc,
                15 + (encoding == "gzip" ? 16 : 0),
                8,
                body);

            if(transfer == "chunked")
            {
                msg += "Transfer-Encoding: chunked\r\n";
                msg += "\r\n";
                append_chunked(msg, deflated_body);
            }
            else if(transfer == "sized")
            {
                msg += "Content-Length: " +
                    std::to_string(deflated_body.size()) + "\r\n";
                msg += "\r\n";
                msg += deflated_body;
            }
            else // to_eof
            {
                msg += "\r\n";
                msg += deflated_body;
            }

            pr.start();
            pr.set_body_limit(body_size);

            auto rs = receiver(
                pr,
                buffers::const_buffer(
                    msg.data(), msg.size()));

            BOOST_TEST(rs == body);

            if(transfer == "to_eof")
                pr.reset();
        }
    }

    void run()
    {
    #ifdef BOOST_RTS_HAS_ZLIB
        test_serializer();
        test_parser();
    #endif
    }
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.zlib");

} // namespace http_proto
} // namespace boost
