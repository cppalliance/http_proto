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
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer_span.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <random>

#include <zlib.h>

void*
zalloc_impl(
    void* /* opaque */,
    unsigned items,
    unsigned size)
{
    try {
        return ::operator new(items * size);
    } catch(std::bad_alloc const&) {
        return Z_NULL;
    }
}

void
zfree_impl(void* /* opaque */, void* addr)
{
    ::operator delete(addr);
}

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
    void verify_compressed(
        span<unsigned char> compressed,
        core::string_view expected)
    {
        int ret = -1;

        z_stream inflate_stream;
        inflate_stream.zalloc = &zalloc_impl;
        inflate_stream.zfree = &zfree_impl;
        inflate_stream.opaque = nullptr;

        auto pstream = &inflate_stream;

        inflate_stream.zalloc = &zalloc_impl;
        inflate_stream.zfree = &zfree_impl;
        inflate_stream.opaque = nullptr;

        int const window_bits = 15; // default
        int const enable_zlib_and_gzip = 32;
        ret = inflateInit2(
            pstream, window_bits + enable_zlib_and_gzip);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
            return;

        ret = inflateReset(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
            return;

        std::vector<unsigned char> decompressed_output(
            2 * expected.size(), 0x00);

        pstream->next_in = compressed.data();
        pstream->avail_in =
            static_cast<unsigned>(compressed.size());

        pstream->next_out = decompressed_output.data();
        pstream->avail_out =
            static_cast<unsigned>(decompressed_output.size());

        ret = inflate(pstream, Z_FINISH);
        if(! BOOST_TEST_EQ(ret, Z_STREAM_END) )
        {
            std::cout << pstream->msg << std::endl;
            // return;
        }

        auto n = pstream->next_out - decompressed_output.data();
        core::string_view sv2(
            reinterpret_cast<char const*>(
                decompressed_output.data()), n);
        BOOST_TEST_EQ(sv2.size(), expected.size());
        // BOOST_TEST_EQ(sv2, expected);

        // BOOST_TEST_EQ(n, 0);

        ret = inflateEnd(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
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
    zlib_serializer()
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

    void run()
    {
        zlib_serializer();
    }
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.zlib");

} // namespace http_proto
} // namespace boost

#endif
