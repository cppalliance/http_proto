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

#include "test_suite.hpp"
#include <sstream>

namespace boost {
namespace http_proto {

class token_list_test
{
public:
    template<class T>
    void
    bad(string_view s)
    {
        T r(s);
        BOOST_TEST(! r.is_valid());
        BOOST_TEST_THROWS(r.validate(),
            system_error);
    }

    template<class T>
    static
    void
    good(
        string_view s,
        string_view match)
    {
        T r(s);
        BOOST_TEST_NO_THROW(r.validate());
        if(! BOOST_TEST(r.is_valid()))
            return;
        std::stringstream ss;
        for(auto it = r.begin(),
            end = r.end();
            it != end;)
        {
            ss << *it++;
            if(it != end)
                ss << ',';
        }
        BOOST_TEST(ss.str() == match);
    }

    void
    testTokenList()
    {
        using T = token_list;

        bad<T>("");
        bad<T>(" ");
        bad<T>("\t");
        bad<T>(",");
        bad<T>(",,");
        bad<T>(", ");
        bad<T>(", ,");
        bad<T>(" x");
        bad<T>("x ");
        bad<T>("x,@");

        good<T>("x", "x");
        good<T>(",x", "x");
        good<T>(", x", "x");
        good<T>(",\tx", "x");
        good<T>("x,", "x");
        good<T>("x,y", "x,y");
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
