//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/chunk_part.hpp>

#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_rule.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(
    is_element<chunk_part>::value);

class chunk_part_test
{
public:
    void
    run()
    {
        {
            using T = chunk_part;
            test::bad<T>( "");
            test::bad<T>( "1\r\n*\r\n");
            test::bad<T>( "1;x\r\n*\r\n");
            test::bad<T>( "10000000000000000\r\n*");
            test::good<T>("0\r\n\r\n");
            test::good<T>("0;x\r\n\r\n");
            test::good<T>("0;x=y\r\n\r\n");
            test::good<T>("0;x=y\r\nx: y\r\n\r\n");
            test::good<T>("1\r\n*");
            test::good<T>("1;x\r\n*");
            test::good<T>("3;x\r\n***");
            test::good<T>("3\r\n*");
        }
        {
            using T = chunk_part_next;
            test::bad<T>( "");
            test::bad<T>( "1\r\n*");
            test::bad<T>( "1;x\r\n*");
            test::bad<T>( "\r\n10000000000000000\r\n*");
            test::good<T>("\r\n0\r\n\r\n");
            test::good<T>("\r\n0;x\r\n\r\n");
            test::good<T>("\r\n0;x=y\r\n\r\n");
            test::good<T>("\r\n0;x=y\r\nx: y\r\n\r\n");
            test::good<T>("\r\n1\r\n*");
            test::good<T>("\r\n1;x\r\n*");
            test::good<T>("\r\n3;x\r\n***");
            test::good<T>("\r\n3\r\n*");
        }
    }
};

TEST_SUITE(chunk_part_test,
    "boost.http_proto.chunk_part");

} // bnf
} // http_proto
} // boost
