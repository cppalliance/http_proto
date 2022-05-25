//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/serializer.hpp>

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/request.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

//------------------------------------------------

class serializer_test
{
public:
    template<class Buffer>
    std::size_t
    write(Buffer const&, error_code&)
    {
        return {};
    }

    void
    testPrototypes()
    {
#if 0
        // send memory-mapped file
        // send serialized json::value


        // entire body in serializer buffer
        // buffer-at-a-time in serializer buffer
        // entire body in external buffer


#endif
    }

    void
    run()
    {
        //testPrototypes();

        context ctx;
        serializer sr;
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost
