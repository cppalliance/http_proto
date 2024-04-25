#include <boost/http_proto/error.hpp>

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

#include <zlib.h>

namespace boost {
namespace http_proto {

void* zalloc(
    void* /* opaque */,
    unsigned items,
    unsigned size)
{
    try {
        return ::operator new(items * size);
    } catch(...) {
        return Z_NULL;
    }
}

void zfree(void* /* opaque */, void* addr)
{
    ::operator delete(addr);
}

struct zlib_filter
{
    z_stream stream_;

    zlib_filter()
    {
        int ret = -1;

        stream_.zalloc = &zalloc;
        stream_.zfree = &zfree;
        stream_.opaque = nullptr;

        ret = deflateInit(&stream_, Z_DEFAULT_COMPRESSION);
        if( ret != Z_OK )
            throw ret;
    }

    ~zlib_filter()
    {
        deflateEnd(&stream_);
    }
};

struct zlib_test
{
    void zlib_hello_world()
    {
        auto string_to_hex = [](core::string_view input)
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

        int ret = -1;

        std::string const msg =
            "hello world, compression seems super duper cool! hmm, but what if I also add like a whole bunch of text to this thing????";

        z_stream stream;
        stream.zalloc = &zalloc;
        stream.zfree = &zfree;
        stream.opaque = nullptr;

        auto pstream = &stream;

        std::vector<unsigned char> input(
            msg.begin(), msg.end());

        // this represents our current serializer approach
        std::vector<unsigned char> workspace(
            std::min(input.size() / 4, std::size_t{512}));

        buffers::circular_buffer tmp0(
            workspace.data(), workspace.size());

        // fixed-sized output buffer that'd represent out
        // zlib filter type
        std::vector<unsigned char> filter_buf(
            16, 0x00);

        ret = deflateInit(pstream, Z_DEFAULT_COMPRESSION);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
          return;

        ret = deflateReset(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
          return;

        std::vector<unsigned char> output(
            2 * deflateBound(pstream, input.size()), 0x00);

        auto out = output.begin();

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
        BOOST_TEST_EQ(n, out - output.begin());

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

        stream.zalloc = &zalloc;
        stream.zfree = &zfree;
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
        pstream->avail_in = out - output.begin();

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

        ret = inflateEnd(pstream);
        if(! BOOST_TEST_EQ(ret, Z_OK) )
          return;
    }

    void run()
    {
        zlib_hello_world();
    }
};

TEST_SUITE(
    zlib_test,
    "boost.http_proto.zlib");

} // namespace http_proto
} // namespace boost
