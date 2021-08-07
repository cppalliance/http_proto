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

#include <boost/http_proto/detail/rfc7230.hpp>

namespace boost {
namespace http_proto {

class ctype_test
{
public:
    template<class T>
    void
    check_set(T cs, string_view s)
    {
        int n = 0;
        unsigned char u = 0;
        do
        {
            char const c =
                static_cast<char>(u);
            if(cs.contains(c))
                ++n;
        }
        while(++u != 0);
        BOOST_TEST(n == s.size());
        for(char c : s)
            BOOST_TEST(cs.contains(c));
    }

    void
    testCharSets()
    {
        check_set(digit_set(),
            "0123456789");

        check_set(tchar_set(),
            "!#$%&'*+-.^_`|~"
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz");
    }

    void
    run()
    {
        testCharSets();
    }
};

TEST_SUITE(ctype_test, "boost.http_proto.ctype");

} // http_proto
} // boost
