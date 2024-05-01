#include <boost/http_proto/error.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/response.hpp>

#include "test_suite.hpp"

#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <zlib.h>

auto string_to_hex = [](boost::core::string_view input)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input)
    {
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
};

namespace boost {
namespace http_proto {

struct zlib_test
{
    void zlib_hello_world()
    {


        (void)string_to_hex;

        int ret = -1;

        // std::filesystem::path input_file("/home/exbigboss/cpp/boost-root/rfc9112");
        // std::string msg(std::filesystem::file_size(input_file), 0x00);
        // std::ifstream ifs(input_file);
        // ifs.read(msg.data(), msg.size());

        std::string msg =
            "hello world, compression seems super duper cool! hmm, but what if I also add like a whole bunch of text to this thing????";

        z_stream stream;
        stream.zalloc = &zalloc_impl;
        stream.zfree = &zfree_impl;
        stream.opaque = nullptr;

        auto pstream = &stream;

        std::vector<unsigned char> input;
        input.reserve(4 * msg.size());
        input.resize(msg.size(), 0x00);
        {
            auto out = input.begin();
            for( auto pos = msg.data(); pos < msg.data() + msg.size(); ++pos )
                *out++ = *pos;
        }

        // this represents our current serializer approach
        std::vector<unsigned char> workspace(
            std::min(input.size() / 4, std::size_t{512}));

        buffers::circular_buffer tmp0(
            workspace.data(), workspace.size());

        // fixed-sized output buffer that'd represent out
        // zlib filter type
        std::vector<unsigned char> filter_buf(16, 0x00);

        ret = deflateInit(pstream, Z_DEFAULT_COMPRESSION);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
          return;

        ret = deflateReset(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
          return;

        std::vector<unsigned char> output(
            2 * deflateBound(pstream, input.size()), 0x00);

        auto out = output.data();

        BOOST_TEST_GT(output.size(), filter_buf.size());

        ret = deflateReset(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
            return;

        pstream->next_out = filter_buf.data();
        pstream->avail_out = filter_buf.size();

        auto append = [&]
        {
            auto begin = filter_buf.data();
            auto end = pstream->next_out;
            // BOOST_ASSERT((end - begin) > 6);

            // keep this in because it suppresses a codegen bug in clang
            std::cout << "out: " << static_cast<void*>( out ) << std::endl;
            std::cout << "[" << static_cast<void*>( begin ) << ", "
                      << static_cast<void*>( end ) << "]" << std::endl;

            for( auto pos = begin; pos < end; ++pos )
                *out++ = *pos;
        };

        span<unsigned char const> input_view = input;
        while(! input_view.empty() )
        {
            auto mbs = tmp0.prepare(tmp0.capacity());
            auto copied =
                buffers::buffer_copy(
                    mbs,
                    buffers::make_buffer(
                        input_view.data(),
                        input_view.size()));

            tmp0.commit(copied);

            BOOST_TEST_GT(copied, 0);

            auto cbs = tmp0.data();
            BOOST_TEST_EQ(
                buffers::buffer_size(cbs), copied);

            for (auto buf : cbs)
            {
                if( buf.size() == 0 )
                    continue;

                pstream->next_in =
                    reinterpret_cast<unsigned char*>(
                        const_cast<void*>(
                            buf.data()));
                pstream->avail_in = buf.size();

                while( pstream->avail_in > 0 )
                {
                    ret = deflate(pstream, Z_NO_FLUSH);
                    BOOST_ASSERT(BOOST_TEST_EQ(ret, Z_OK));
                    if( pstream->avail_out == 0 )
                    {
                        // BOOST_ASSERT(false);
                        ret = deflate(pstream, Z_SYNC_FLUSH);
                        append();
                        pstream->next_out = filter_buf.data();
                        pstream->avail_out = filter_buf.size();

                        while(pstream->avail_out == 0)
                        {
                            pstream->next_out = filter_buf.data();
                            pstream->avail_out = filter_buf.size();
                            ret = deflate(pstream, Z_SYNC_FLUSH);
                            append();
                            pstream->next_out = filter_buf.data();
                            pstream->avail_out = filter_buf.size();
                        }
                    }
                }
            }

            ret = deflate(pstream, Z_SYNC_FLUSH);
            append();
            pstream->next_out = filter_buf.data();
            pstream->avail_out = filter_buf.size();

            while(pstream->avail_out == 0)
            {
                pstream->next_out = filter_buf.data();
                pstream->avail_out = filter_buf.size();
                ret = deflate(pstream, Z_SYNC_FLUSH);
                append();
                pstream->next_out = filter_buf.data();
                pstream->avail_out = filter_buf.size();
            }

            tmp0.consume(copied);
            input_view = input_view.subspan(copied);
        }

        BOOST_TEST_EQ(pstream->total_in, msg.size());

        if( pstream->avail_out != filter_buf.size() ) {
            append();

            pstream->next_out = filter_buf.data();
            pstream->avail_out = filter_buf.size();
        }

        do {
            pstream->next_out = filter_buf.data();
            pstream->avail_out = filter_buf.size();
            ret = deflate(pstream, Z_FINISH);
            append();
        } while(pstream->avail_out == 0);

        auto n = pstream->total_out;
        BOOST_TEST_EQ(n, out - output.data());
        // BOOST_TEST_EQ(n, 0);

        ret = deflateEnd(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
        {
          return;
        }

        // BOOST_TEST_EQ(
        //     string_to_hex(
        //         core::string_view(
        //             reinterpret_cast<char const*>(
        //                 output.data()),
        //                 out - output.begin())),
        //     "");

        //--------------------------------------------------

        // for( auto& c : input )
        //     c = 0;

        stream.zalloc = &zalloc_impl;
        stream.zfree = &zfree_impl;
        stream.opaque = nullptr;

        ret = inflateInit(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
            return;

        ret = inflateReset(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
            return;

        std::vector<unsigned char> decompressed_output(
            2 * input.size(), 0x00);

        pstream->next_in = output.data();
        pstream->avail_in = out - output.data();

        pstream->next_out = decompressed_output.data();
        pstream->avail_out = decompressed_output.size();

        ret = inflate(pstream, Z_FINISH);
        if(! BOOST_TEST_EQ(ret, Z_STREAM_END) )
        {
            std::cout << pstream->msg << std::endl;
            // return;
        }

        n = pstream->next_out - decompressed_output.data();
        core::string_view sv(reinterpret_cast<char const*>(decompressed_output.data()), n);
        BOOST_TEST_EQ(sv, msg);

        // BOOST_TEST_EQ(n, 0);

        ret = inflateEnd(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
          return;
    }

    void
    zlib_serializer()
    {
        std::cout << "zlib_serializer()" << std::endl;

        zlib_filter zfilter;

        core::string_view str =
            "HTTP/1.1 200 OK\r\n"
            "Content-Encoding: deflate\r\n"
            "\r\n";

        std::string const body =
            "hello world, compression seems super duper cool! hmm, but what if I also add like a whole bunch of text to this thing????";

        span<char const> body_view = body;

        response res(str);

        std::vector<unsigned char> output(
            str.size() + body.size(), 0x00);

        buffers::mutable_buffer output_buf(
            output.data(), output.size());

        serializer sr(1024);
        sr.is_compressed_ = true;
        sr.zlib_filter_ = &zfilter;

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

            BOOST_TEST_EQ(n, body.size());

            stream.commit(n);

            auto cbs = sr.prepare().value();
            BOOST_TEST_EQ(
                buffers::buffer_size(cbs), 0);

            auto n2 = buffers::buffer_copy(
                output_buf, cbs);

            sr.consume(n2);
            output_buf += n2;

            body_view = body_view.subspan(n);
        }
        stream.close();

        auto cbs = sr.prepare().value();
        BOOST_TEST_EQ(
            buffers::buffer_size(cbs), 0);

        output_buf += buffers::buffer_copy(
                output_buf, cbs);

        auto m = output.size() - output_buf.size();
        span<unsigned char> compressed(output.data(), m);

        auto sv =
            core::string_view(
                    reinterpret_cast<char const*>(
                        compressed.data()),
                        compressed.size());

        BOOST_TEST(sv.starts_with(str));

        sv = sv.substr(str.size());

        compressed = compressed.subspan(str.size());
        BOOST_TEST_EQ(compressed.size(), 100);
        BOOST_TEST_EQ(
            string_to_hex(sv), "");

        {
            int ret = -1;

            z_stream stream;
            stream.zalloc = &zalloc_impl;
            stream.zfree = &zfree_impl;
            stream.opaque = nullptr;

            auto pstream = &stream;

            stream.zalloc = &zalloc_impl;
            stream.zfree = &zfree_impl;
            stream.opaque = nullptr;

            ret = inflateInit(pstream);
            if(! BOOST_TEST_EQ(ret, Z_OK) )
                return;

            ret = inflateReset(pstream);
            if(! BOOST_TEST_EQ(ret, Z_OK) )
                return;

            std::vector<unsigned char> decompressed_output(
                2 * body.size(), 0x00);

            pstream->next_in = compressed.data();
            pstream->avail_in = compressed.size();

            pstream->next_out = decompressed_output.data();
            pstream->avail_out = decompressed_output.size();

            ret = inflate(pstream, Z_FINISH);
            if(! BOOST_TEST_EQ(ret, Z_STREAM_END) )
            {
                std::cout << pstream->msg << std::endl;
                // return;
            }

            auto n = pstream->next_out - decompressed_output.data();
            core::string_view sv(reinterpret_cast<char const*>(decompressed_output.data()), n);
            BOOST_TEST_EQ(sv, body);

            // BOOST_TEST_EQ(n, 0);

            ret = inflateEnd(pstream);
            if(! BOOST_TEST_EQ(ret, Z_OK) )
            return;
        }
    }

    void run()
    {
        zlib_hello_world();
        zlib_serializer();
    }
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.zlib");

} // namespace http_proto
} // namespace boost
