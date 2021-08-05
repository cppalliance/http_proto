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
#include <sstream>

namespace boost {
namespace http_proto {

class token_list_test
{
public:
    template<class T>
    static
    void
    check(
        string_view s,
        string_view good)
    {
        T list(s);
        std::stringstream ss;
        for(auto it = list.begin(),
            end = list.end();
            it != end;)
        {
            ss << *it++;
            if(it != end)
                ss << ',';
        }
        BOOST_TEST(ss.str() == good);
    }

    void
    testTokenList()
    {
        using T = token_list;
        check<T>("a", "a");
        check<T>("ab", "ab");
        check<T>("abc", "abc");
        check<T>("a,b", "a,b");
        check<T>("a,b,c", "a,b,c");
        check<T>(",a", "a");
        check<T>("a,", "a");
        check<T>(",a,", "a");
        check<T>(",,a,", "a");
        check<T>(",a,,", "a");
    }

    void
    run()
    {
        testTokenList();
    }
};

TEST_SUITE(token_list_test, "boost.http_proto.token_list");

} // http_proto
} // boost
