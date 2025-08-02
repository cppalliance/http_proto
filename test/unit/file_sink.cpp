//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/file_sink.hpp>

#include <boost/filesystem.hpp>
#include <fstream>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct file_sink_test
{
    class temp_path
    {
        std::string path_;

    public:
        temp_path()
            // filesystem::path::string() fails on older
            // versions of mingw when rtti is off
            : path_(filesystem::unique_path().string())
        {
        }

        operator char const*() const noexcept
        {
            return path_.c_str();
        }

        ~temp_path()
        {
            filesystem::remove(path_);
        }
    };

    std::string
    read_file(temp_path const& path)
    {
        std::ifstream ifs(path);
        std::ostringstream buf;
        buf << ifs.rdbuf();
        return buf.str();
    }

    void
    testReportErros()
    {
        // passing a closed file
        {
            file f;
            file_sink fsink(std::move(f));
            buffers::const_buffer cb("123", 3);
            auto rs = fsink.write(cb, true);
            BOOST_TEST_EQ(rs.bytes, 0);
            BOOST_TEST(rs.ec.failed());
        }
    }

    void
    testWrite()
    {
        temp_path path;
        file f;
        system::error_code ec;
        f.open(path, file_mode::write, ec);
        BOOST_TEST(!ec);
        BOOST_TEST(f.is_open());
        file_sink fsink(std::move(f));
        std::array<core::string_view, 4> bufs{
            "Hello",
            ", ",
            "World!",
            {} }; // empty
        for(auto s : bufs)
        {
            buffers::const_buffer cb(s.data(), s.size());
            auto rs = fsink.write(cb, s.size() != 0);
            BOOST_TEST_EQ(rs.bytes, cb.size());
            BOOST_TEST(!rs.ec);
        }
        BOOST_TEST_EQ(
            read_file(path),
            "Hello, World!");
    }

    void
    run()
    {
        testReportErros();
        testWrite();
    }
};

TEST_SUITE(
    file_sink_test,
    "boost.http_proto.file_sink");

} // http_proto
} // boost
