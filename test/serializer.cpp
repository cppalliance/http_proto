//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/serializer.hpp>

#include <boost/http_proto/context.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct file_body
{
    class parser
    {
    };

    class serializer
    {
    };
};

struct string_body
{
    using value_type = std::string;


};

class serializer_test
{
public:
    void
    run()
    {
        context ctx;
        serializer sr(ctx);
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost
#if 0

class buffered_body
{
public:
    virtual std::size_t write(
        void* dest,
        std::size_t bytes,
        error_code& ec) = 0;
};
#endif
