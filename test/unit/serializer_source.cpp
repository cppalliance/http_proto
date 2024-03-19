//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

// Test that header file is self-contained.
#include <boost/http_proto/serializer.hpp>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

struct serializer_source_test
{
    struct test_source : serializer::source
    {
        buffers::const_buffer cb_;
        std::size_t fail_;

        explicit
        test_source(
            std::size_t fail) noexcept
            : fail_(fail)
        {
            auto const& pat = test_pattern();
            cb_ = { &pat[0], pat.size() };
        }

        results
        on_read(
            buffers::mutable_buffer b) override
        {
            results rv;
            if(fail_-- == 0)
            {
                rv.ec = boost::system::error_code(
                    boost::system::errc::invalid_argument,
                    boost::system::generic_category());
                return rv;
            }
            auto const n =
                buffers::buffer_copy(b, cb_);
            cb_ += n;
            rv.bytes += n;
            rv.finished = cb_.size() == 0;
            return rv;
        }
    };

    void
    testSource()
    {
        auto const& pat = test_pattern();

        // read(MutableBufferSequence)
        for(std::size_t i = 0;;++i)
        {
            test_source src(i);
            std::string s(
                pat.size(), 0);
            buffers::mutable_buffer mb[3] = {
                { &s[0], 3 },
                { &s[3], 5 },
                { &s[8], 7 } };
            buffers::mutable_buffer_span bs(mb, 3);
            auto rv = src.read(bs);
            if(rv.ec.failed())
                continue;
            BOOST_TEST(rv.finished);
            BOOST_TEST_EQ(
                rv.bytes, pat.size());
            s.resize(rv.bytes);
            BOOST_TEST_EQ(s, pat);
            break;
        }

        // read(mutable_buffer)
        for(std::size_t i = 0;;++i)
        {
            test_source src(i);
            std::string s(
                pat.size(), 0);
            buffers::mutable_buffer mb(
                &s[0], s.size());
            auto rv = src.read(mb);
            if(rv.ec.failed())
                continue;
            BOOST_TEST(rv.finished);
            BOOST_TEST_EQ(
                rv.bytes, pat.size());
            s.resize(rv.bytes);
            BOOST_TEST_EQ(s, pat);
            break;
        }

        // empty sequence
        {
            test_source src(99);
            buffers::mutable_buffer_span bs;
            auto rv = src.read(bs);
            BOOST_TEST(! rv.ec.failed());
            BOOST_TEST_EQ(rv.bytes, 0);
        }
    }

    void
    run()
    {
        testSource();
    }
};

TEST_SUITE(
    serializer_source_test,
    "boost.http_proto.serializer.source");

} // http_proto
} // boost
