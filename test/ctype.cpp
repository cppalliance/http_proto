//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/ctype.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class ctype_test
{
public:
    void
    test_ctype(
        bool (*pf)(char),
        string_view set)
    {
        unsigned char u = 0;
        do
        {
            char const c = static_cast<char>(u);
            if(set.find(c) != string_view::npos)
                BOOST_TEST(pf(c));
            else
                BOOST_TEST(! pf(c));
        }
        while(++u != 0);
    }

    void
    test_is_tchar()
    {
        test_ctype(
            &is_tchar,
            "!#$" "%%" "&'*+-.^_`|~"
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
        );
    }

    void
    run()
    {
        test_is_tchar();
    }
};

TEST_SUITE(ctype_test, "boost.http_proto.ctype");

} // http_proto
} // boost
