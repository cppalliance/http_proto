//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/chunk_ext.hpp>

#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(
    is_list<chunk_ext>::value);

class chunk_ext_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(chunk_ext_test, "boost.http_proto.chunk_ext");

} // bnf
} // http_proto
} // boost
