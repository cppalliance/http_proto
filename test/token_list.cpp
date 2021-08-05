//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/token_list.hpp>

#include <boost/http_proto/forward_range.hpp>

#include "test_suite.hpp"
#include <iostream>

namespace boost {
namespace http_proto {

class token_list_test
{
public:
    void
    run()
    {
        test_suite::debug_stream log(std::cout);

        string_view in =
            "close,, ,keep-alive, upgrade,  ";

        for(auto v : token_list(in))
            log << '\"' << v << '\"' << std::endl;
    }
};

TEST_SUITE(token_list_test, "boost.http_proto.token_list");

} // http_proto
} // boost
