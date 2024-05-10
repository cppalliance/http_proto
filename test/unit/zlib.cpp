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
// #include <filesystem>
#include <fstream>

#include <zlib.h>

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

        // `+ 32` => enable zlib + gzip parsing
        ret = inflateInit2(pstream, 15 + 32);
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
        BOOST_TEST_EQ(sv2, expected);

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
        struct sink : public source{
            span<char const>& body_view_;
            sink(span<char const>& bv) : body_view_(bv) {}

            results on_read(buffers::mutable_buffer b) {
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

        while(! body_view.empty() )
        {
            auto cbs = sr.prepare().value();
            BOOST_TEST_GT(buffers::buffer_size(cbs), 0);

            auto n2 = buffers::buffer_copy(
                output_buf, cbs);
            BOOST_TEST_EQ(n2, buffers::buffer_size(cbs));
            sr.consume(n2);
            output_buf += n2;
        }

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

    using fp_type =
        buffers::mutable_buffer(*)(
            response_view,
            serializer&,
            span<char const>,
            span<unsigned char>);

    void zlib_serializer_impl(fp_type fp)
    {
        std::cout << "zlib_serializer()" << std::endl;

        zlib_filter zfilter;

        // std::filesystem::path input_file("/home/exbigboss/cpp/boost-root/rfc9112");
        // std::string body(std::filesystem::file_size(input_file), 0x00);
        // std::ifstream ifs(input_file);
        // ifs.read(body.data(), body.size());

        std::string const body =
            "hello world, compression seems super duper cool! hmm, but what if I also add like a whole bunch of text to this thing????";

        span<char const> body_view = body;

        content_coding_type c = content_coding_type::gzip;

        core::string_view str;
        if( c == content_coding_type::deflate )
            str =
                "HTTP/1.1 200 OK\r\n"
                "Content-Encoding: deflate\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n";
        if( c == content_coding_type::gzip )
            str =
                "HTTP/1.1 200 OK\r\n"
                "Content-Encoding: gzip\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n";

        response res;
        res.set_content_encoding(c);
        res.set_chunked(true);

        serializer sr(1024);
        sr.zlib_filter_ = &zfilter;

        std::vector<unsigned char> output(
            str.size() + 2 * body.size(), 0x00);

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

        // BOOST_TEST_EQ(
        //     string_to_hex(sv), "");

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
        (void)string_to_hex;

        auto safe_print = [](core::string_view d)
        {
            for(auto c : d)
            {
                if( c == '\r' ) std::cout << "\\r";
                else if( c == '\n' ) std::cout << "\\n";
                else if( c < 31 ) std::cout << 'X';
                else std::cout << c;
            }
            std::cout << std::endl;
        };
        (void) safe_print;

        std::vector<unsigned char> compressed;

        while(! sv.empty() )
        {
            if(! res.chunked() )
            {
                compressed.insert(
                    compressed.end(), sv.begin(), sv.end());
                break;
            }

            // safe_print(sv);

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
                compressed.insert(
                    compressed.end(),
                    chunk.begin(), chunk.begin() + chunk_size);
                chunk.remove_prefix(chunk_size);
                BOOST_TEST(chunk.starts_with("\r\n"));
            }
            chunk.remove_prefix(2);
        }

        verify_compressed(compressed, body);
    }

    void
    zlib_serializer()
    {
        zlib_serializer_impl(zlib_serializer_stream);
        zlib_serializer_impl(zlib_serializer_source);
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
