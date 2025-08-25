//
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/serializer.hpp>

#include <boost/buffers.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>
#include <boost/rts/brotli.hpp>
#include <boost/rts/context.hpp>
#include <boost/rts/zlib.hpp>

#include "test_helpers.hpp"

#include <string>
#include <vector>
#include <random>

namespace boost {
namespace http_proto {

struct zlib_test
{
    static
    std::string
    make_rand_string(std::size_t length)
    {
        const char chars[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<std::size_t> dist(0, sizeof(chars) - 2);

        std::string result;
        result.resize(length);
        std::generate(
            result.begin(),
            result.end(),
            [&]() { return chars[dist(rng)]; });

        return result;
    }

    static
    std::string
    compress(
        const rts::context& ctx,
        core::string_view encoding,
        core::string_view body)
    {
        std::string result;
        auto buf = buffers::string_buffer(&result);

        if(encoding == "deflate" || encoding == "gzip")
        {
            namespace zlib = rts::zlib;
            auto& svc = ctx.get_service<zlib::deflate_service>();
            zlib::stream zs{};

            auto ret = static_cast<zlib::error>(
                svc.init2(
                    zs,
                    zlib::default_compression,
                    zlib::deflated,
                    encoding == "deflate" ? 15 : 31,
                    8,
                    zlib::default_strategy));

            if(!BOOST_TEST_EQ(ret, zlib::error::ok))
            {
                svc.deflate_end(zs);
                return {};
            }

            zs.next_in  = reinterpret_cast<unsigned char*>(const_cast<char *>(body.data()));
            zs.avail_in = static_cast<unsigned int>(body.size());

            for(;;)
            {
                zs.next_out  = reinterpret_cast<unsigned char*>(buf.prepare(64 * 1024).data());
                zs.avail_out = 64 * 1024;
                ret = static_cast<zlib::error>(svc.deflate(zs, zlib::finish));
                buf.commit(64 * 1024 - zs.avail_out);
                if(ret != zlib::error::ok)
                    break;
            }

            svc.deflate_end(zs);
        }
        else if(encoding == "br")
        {
            namespace brotli = rts::brotli;
            auto& svc = ctx.get_service<brotli::encode_service>();

            brotli::encoder_state* state =
                svc.create_instance(nullptr, nullptr, nullptr);

            if(!BOOST_TEST_NE(state, nullptr))
                return {};

            auto* next_in = reinterpret_cast<const std::uint8_t*>(body.data());
            auto available_in = body.size();

            svc.set_parameter(
                state, brotli::encoder_parameter::quality, 5);
            do
            {
                auto* next_out = reinterpret_cast<std::uint8_t*>(buf.prepare(64 * 1024).data());
                std::size_t available_out = 64 * 1024;
                auto ret = svc.compress_stream(
                    state,
                    brotli::encoder_operation::finish,
                    &available_in,
                    &next_in,
                    &available_out,
                    &next_out,
                    nullptr);
                buf.commit(64 * 1024 - available_out);
                if(!BOOST_TEST_EQ(ret, true))
                    break;
            } while(!svc.is_finished(state));

            svc.destroy_instance(state);
        }
        else
        {
            BOOST_TEST_FAIL();
        }

        result.resize(buf.size());
        return result;
    }

    static
    void
    verify_compressed(
        const rts::context& ctx,
        core::string_view encoding,
        core::string_view compressed_body,
        core::string_view body)
    {
        if(encoding == "deflate" || encoding == "gzip")
        {
            namespace zlib = rts::zlib;
            auto& svc = ctx.get_service<zlib::inflate_service>();
            zlib::stream zs{};

            auto ret = static_cast<zlib::error>(
                svc.init2(
                    zs,
                    encoding == "deflate" ? 15 : 31));
            if(!BOOST_TEST_EQ(ret, zlib::error::ok))
            {
                svc.inflate_end(zs);
                return;
            }

            zs.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(compressed_body.data()));
            zs.avail_in = static_cast<unsigned>(compressed_body.size());

            for(;;)
            {
                char buf[64 * 1024];
                zs.next_out = reinterpret_cast<unsigned char*>(&buf[0]);
                zs.avail_out = 64 * 1024;
                ret = static_cast<zlib::error>(svc.inflate(zs, zlib::finish));
                auto piece = core::string_view{ &buf[0], 64 * 1024 - zs.avail_out };
                if(!BOOST_TEST(body.starts_with(piece)))
                    break;
                body.remove_prefix(piece.size());
                if(ret == zlib::error::stream_end)
                    break;
                if(ret == zlib::error::buf_err && zs.avail_out == 0)
                    continue;
                if(!BOOST_TEST_GE(static_cast<int>(ret), 0))
                    break;
            };

            svc.inflate_end(zs);
        }
        else if(encoding == "br")
        {
            namespace brotli = rts::brotli;
            auto& svc = ctx.get_service<brotli::decode_service>();

            brotli::decoder_state* state =
                svc.create_instance(nullptr, nullptr, nullptr);

            if(!BOOST_TEST_NE(state, nullptr))
                return;

            auto* next_in = reinterpret_cast<const std::uint8_t*>(compressed_body.data());
            auto available_in = compressed_body.size();

            for(;;)
            {
                char buf[64 * 1024];
                auto* next_out =reinterpret_cast<std::uint8_t*>(&buf[0]);
                std::size_t available_out = 64 * 1024;
                brotli::decoder_result ret = svc.decompress_stream(
                    state,
                    &available_in,
                    &next_in,
                    &available_out,
                    &next_out,
                    nullptr);
                auto piece = core::string_view{ &buf[0], 64 * 1024 - available_out };
                if(!BOOST_TEST(body.starts_with(piece)))
                    break;
                body.remove_prefix(piece.size());
                if(svc.is_finished(state))
                    break;
                if(!BOOST_TEST_NE(ret, brotli::decoder_result::error))
                    break;
            };

            svc.destroy_instance(state);
        }
        else
        {
            BOOST_TEST_FAIL();
        }

        BOOST_TEST(body.empty());
    }

    static
    void
    serializer_source(
        response_view res,
        serializer& sr,
        buffers::const_buffer body,
        buffers::string_buffer out)
    {
        class source_t : public source
        {
            buffers::const_buffer body_;
            bool done_ = false;

        public:
            source_t(buffers::const_buffer body)
                : body_(body)
            {
            }

            results
            on_read(buffers::mutable_buffer b)
            {
                BOOST_TEST_NOT(done_);

                results rs;
                auto n = buffers::copy(b, body_);
                body_ = buffers::sans_prefix(body_, n);
                rs.bytes = n;
                rs.finished = (body_.size() == 0);
                done_ = rs.finished;
                return rs;
            }
        };

        sr.start<source_t>(res, body);
        do
        {
            auto cbs = sr.prepare();
            auto n = buffers::size(cbs.value());
            BOOST_TEST_GT(n, 0);
            buffers::copy(out.prepare(n), cbs.value());
            sr.consume(n);
            out.commit(n);
        } while(!sr.is_done());
    }

    static
    void
    serializer_stream(
        response_view res,
        serializer& sr,
        buffers::const_buffer body,
        buffers::string_buffer out)
    {
        auto stream = sr.start_stream(res);
        do
        {
            if(stream.is_open())
            {
                auto mbs = stream.prepare();
                auto n = buffers::copy(mbs, body);
                body = buffers::sans_prefix(body, n);
                stream.commit(n);
                if(body.size() == 0)
                    stream.close();
            }

            auto cbs = sr.prepare();
            if(cbs.has_error())
            {
                BOOST_ASSERT(
                    cbs.error() == error::need_data);
            }
            else
            {
                auto n = buffers::size(cbs.value());
                BOOST_TEST_GT(n, 0);
                buffers::copy(out.prepare(n), cbs.value());
                sr.consume(n);
                out.commit(n);
            }
        } while(!sr.is_done());
    }

    static
    void
    serializer_buffers(
        response_view res,
        serializer& sr,
        buffers::const_buffer body,
        buffers::string_buffer out)
    {
        std::vector<buffers::const_buffer> buf_seq;
        do
        {
            auto buf_size = std::min(body.size() / 23, body.size());
            if(buf_size == 0)
                buf_size = 1;
            buf_seq.push_back(buffers::prefix(body, buf_size));
            body = buffers::sans_prefix(body, buf_size);
        } while(body.size() != 0);

        sr.start(res, buf_seq);
        do
        {
            auto cbs = sr.prepare();
            auto n = buffers::size(cbs.value());
            BOOST_TEST_GT(n, 0);
            buffers::copy(out.prepare(n), cbs.value());
            sr.consume(n);
            out.commit(n);
        }while(!sr.is_done());
    }

    static
    void
    serializer_empty(
        response_view res,
        serializer& sr,
        buffers::const_buffer body,
        buffers::string_buffer out)
    {
        BOOST_TEST(body.size() == 0);
        // empty body
        sr.start(res);
        do
        {
            auto cbs = sr.prepare();
            auto n = buffers::size(cbs.value());
            BOOST_TEST_GT(n, 0);
            buffers::copy(out.prepare(n), cbs.value());
            sr.consume(n);
            out.commit(n);
        }while(!sr.is_done());
    }

    void
    test_serializer()
    {
        rts::context ctx;
        std::vector<std::string> encodings;
        serializer::config cfg;

        #ifdef BOOST_RTS_HAS_ZLIB
            cfg.apply_deflate_encoder = true;
            cfg.apply_gzip_encoder = true;
            rts::zlib::install_deflate_service(ctx);
            rts::zlib::install_inflate_service(ctx);
            encodings.push_back("gzip");
            encodings.push_back("deflate");
        #endif
        #ifdef BOOST_RTS_HAS_BROTLI
            cfg.apply_brotli_encoder = true;
            rts::brotli::install_encode_service(ctx);
            rts::brotli::install_decode_service(ctx);
            encodings.push_back("br");
        #endif

        install_serializer_service(ctx, cfg);
        serializer sr(ctx);

        const auto rand_string = make_rand_string(1024 * 1024);

        for(core::string_view encoding : encodings) 
        for(auto chunked : { true, false })
        for(auto body_size : { 0, 7, 64 * 1024, 1024 * 1024 })
        for(auto driver : { serializer_empty, serializer_buffers, serializer_stream, serializer_source })
        {
            if(driver == serializer_empty && body_size != 0)
                continue;

            response resp;
            resp.set(field::content_encoding, encoding);
            if(chunked)
                resp.set_chunked(true);
            else
                resp.set_content_length(body_size);

            auto body = core::string_view{ rand_string }.substr(0, body_size);
            std::string buf;
            driver(
                resp,
                sr,
                buffers::const_buffer(body.data(), body.size()),
                buffers::string_buffer(&buf));

            BOOST_TEST(
                core::string_view{ buf }.starts_with(resp.buffer()));

            auto raw_body = core::string_view{ buf }.substr(resp.buffer().size());
            std::string compressed_body;

            while(!raw_body.empty())
            {
                if(!resp.chunked())
                {
                    compressed_body = raw_body;
                    break;
                }

                auto pos = raw_body.find_first_of("\r\n");
                BOOST_TEST_NE(pos, core::string_view::npos);

                std::string chunk_header = raw_body.substr(0, pos);
                raw_body.remove_prefix(pos + 2);

                auto chunk_size = std::stoul(chunk_header, nullptr, 16);

                if(chunk_size == 0)
                {
                    BOOST_TEST(raw_body == "\r\n");
                }
                else
                {
                    BOOST_TEST_LT(
                        raw_body.begin() + chunk_size,
                        raw_body.end());

                    compressed_body.insert(
                        compressed_body.end(),
                        raw_body.data(),
                        raw_body.data() + chunk_size);

                    raw_body.remove_prefix(chunk_size);
                    BOOST_TEST(raw_body.starts_with("\r\n"));
                }
                raw_body.remove_prefix(2);
            }

            verify_compressed(ctx, encoding, compressed_body, body);
        }
    }

    static
    std::string
    parser_pull_body(
        response_parser& pr,
        buffers::const_buffer input)
    {
        std::string rs;
        buffers::string_buffer buf(&rs);
        for(;;)
        {
            if(input.size() != 0)
            {
                auto n1 = buffers::copy(pr.prepare(), input);
                pr.commit(n1);
                input = buffers::sans_prefix(input, n1);
            }

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
    parser_elastic_body(
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
    parser_sink_body(
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
            bool done_ = false;

        public:
            std::string
            get_body()
            {
                return body_;
            }

            results
            on_write(
                buffers::const_buffer b,
                bool more) override
            {
                BOOST_TEST_NOT(done_);
                done_ = !more;

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
        std::vector<std::string> encodings;
        response_parser::config cfg;

        #ifdef BOOST_RTS_HAS_ZLIB
            cfg.apply_deflate_decoder = true;
            cfg.apply_gzip_decoder = true;
            rts::zlib::install_deflate_service(ctx);
            rts::zlib::install_inflate_service(ctx);
            encodings.push_back("gzip");
            encodings.push_back("deflate");
        #endif
        #ifdef BOOST_RTS_HAS_BROTLI
            cfg.apply_brotli_decoder = true;
            rts::brotli::install_encode_service(ctx);
            rts::brotli::install_decode_service(ctx);
            encodings.push_back("br");
        #endif

        cfg.body_limit = 1024 * 1024;
        install_parser_service(ctx, cfg);
        response_parser pr(ctx);
        pr.reset();

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

        const auto rand_string = make_rand_string(1024 * 1024);

        for(core::string_view encoding : encodings) 
        for(core::string_view transfer : { "chunked", "sized", "to_eof" })
        for(auto body_size : { 0, 7, 64 * 1024, 1024 * 1024 })
        for(auto driver : { parser_pull_body, parser_sink_body, parser_elastic_body })
        {
            std::string msg = "HTTP/1.1 200 OK\r\n";
            msg += "Content-Encoding: ";
            msg += encoding;
            msg += "\r\n";

            auto body = core::string_view{ rand_string }.substr(0, body_size);
            auto compressed_body = compress(
                ctx,
                encoding,
                body);

            if(transfer == "chunked")
            {
                msg += "Transfer-Encoding: chunked\r\n";
                msg += "\r\n";
                append_chunked(msg, compressed_body);
            }
            else if(transfer == "sized")
            {
                msg += "Content-Length: " +
                    std::to_string(compressed_body.size()) + "\r\n";
                msg += "\r\n";
                msg += compressed_body;
            }
            else // to_eof
            {
                msg += "\r\n";
                msg += compressed_body;
            }

            pr.start();
            pr.set_body_limit(body_size);

            auto rs = driver(
                pr,
                buffers::const_buffer(msg.data(), msg.size()));

            BOOST_TEST(rs == body);

            if(transfer == "to_eof")
                pr.reset();
        }
    }

    void run()
    {
        test_serializer();
        test_parser();
    }
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.compression");

} // namespace http_proto
} // namespace boost
