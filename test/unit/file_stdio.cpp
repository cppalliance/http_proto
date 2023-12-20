//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/file_stdio.hpp>

#include <boost/config.hpp>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) ||                 \
    defined(__CYGWIN__)
#define BOOST_HTTP_PROTO_IS_WIN
#endif

#if defined(BOOST_NO_RTTI) && defined(BOOST_HTTP_PROTO_IS_WIN) &&              \
    defined(BOOST_GCC)

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE("skipping file_stdio_ tests for this configuration")

#else

#include "file_test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class file_stdio_test
{
public:
    void
    run()
    {
#ifdef BOOST_MSVC
        test_file<file_stdio, true>();
#else
        test_file<file_stdio>();
#endif
    }
};

TEST_SUITE(
    file_stdio_test,
    "boost.http_proto.file_stdio");
} // http_proto
} // boost

#endif
