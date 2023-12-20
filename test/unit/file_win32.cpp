//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/file_win32.hpp>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) ||                 \
    defined(__CYGWIN__)
#define BOOST_HTTP_PROTO_IS_WIN
#endif

#if defined(BOOST_NO_RTTI) && defined(BOOST_HTTP_PROTO_IS_WIN) &&              \
    defined(BOOST_GCC)

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE("skipping file_win32_ tests for this configuration")

#else

#if BOOST_HTTP_PROTO_USE_WIN32_FILE

#include "file_test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class file_win32_test
{
public:
    void
    run()
    {
        test_file<file_win32, true>();
    }
};

TEST_SUITE(
    file_win32_test,
    "boost.http_proto.file_win32");

} // http_proto
} // boost

#endif // BOOST_HTTP_PROTO_USE_WIN32_FILE
#endif
