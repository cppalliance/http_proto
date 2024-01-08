//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

// Test that header file is self-contained.
#include <boost/http_proto/sink.hpp>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

struct sink_test
{
    struct test_sink : sink
    {
        std::string s_;
        std::size_t fail_;

        explicit
        test_sink(
            std::size_t fail) noexcept
            : fail_(fail)
        {
        }

        std::string const&
        str() const noexcept
        {
            return s_;
        }

        results
        on_write(
            buffers::const_buffer b,
            bool) override
        {
            results rv;
            if(fail_-- == 0)
            {
                rv.ec = system::error_code(
                    system::errc::invalid_argument,
                    system::generic_category());
                return rv;
            }
            s_.append(static_cast<
                char const*>(b.data()),
                    b.size());
            rv.bytes += b.size();
            return rv;
        }
    };

    void
    testSink()
    {
        auto const& pat = test_pattern();

        // write(ConstBufferSequence)
        for(std::size_t i = 0;;++i)
        {
            test_sink dest(i);
            buffers::const_buffer cb[3] = {
                { &pat[0], 3 },
                { &pat[3], 5 },
                { &pat[8], 7 } };
            buffers::const_buffer_span bs(cb, 3);
            auto rv = dest.write(bs, false);
            if(rv.ec.failed())
                continue;
            BOOST_TEST_EQ(dest.str(), pat);
            break;
        }

        // write(const_buffer)
        for(std::size_t i = 0;;++i)
        {
            test_sink dest(i);
            buffers::const_buffer cb(
                &pat[0], pat.size());
            auto rv = dest.write(cb, false);
            if(rv.ec.failed())
                continue;
            BOOST_TEST_EQ(dest.str(), pat);
            break;
        }

        // write(mutable_buffer)
        for(std::size_t i = 0;;++i)
        {
            test_sink dest(i);
            std::string s = pat;
            buffers::mutable_buffer mb(
                &s[0], s.size());
            auto rv = dest.write(mb, false);
            if(rv.ec.failed())
                continue;
            BOOST_TEST_EQ(dest.str(), pat);
            break;
        }

        // empty sequence
        {
            test_sink dest(99);
            buffers::const_buffer_span bs;
            auto rv = dest.write(bs, true);
            BOOST_TEST(! rv.ec.failed());
            BOOST_TEST_EQ(rv.bytes, 0);
        }
    }

    void
    run()
    {
        testSink();
    }
};

TEST_SUITE(
    sink_test,
    "boost.http_proto.sink");

} // http_proto
} // boost
