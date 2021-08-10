//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/token_list.hpp>

#include "test_suite.hpp"
#include "test_bnf.hpp"
#include <string_view>

namespace boost {
namespace http_proto {

BOOST_STATIC_ASSERT(
    std::is_trivially_destructible<
        token_list>::value);

class token_list_test
{
public:
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
    run()
    {
        using namespace test;
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
        bad<T>("x, ");

        good<T>("x", "x");
        good<T>(",x", "x");
        good<T>(", x", "x");
        good<T>(",\tx", "x");
        good<T>("x,", "x");
        good<T>("x,y", "x,y");
        good<T>("x, y", "x,y");
        good<T>("a,b,c", "a,b,c");
        good<T>("a ,b,c", "a,b,c");
        good<T>("a, b,c", "a,b,c");
        good<T>("a,b ,c", "a,b,c");
        good<T>("a,b, c", "a,b,c");

        suffix<T>("x", 0);
        suffix<T>("x ", 1);
        suffix<T>("x@", 1);
        suffix<T>("x @", 2);
        suffix<T>("x,", 0);
        suffix<T>("x, ", 1);
        suffix<T>("x, @", 2);
        suffix<T>("x,x", 0);
        suffix<T>("x,x ", 1);
        suffix<T>("x,x x", 2);
        suffix<T>("x,x ,", 0);
    }
};

TEST_SUITE(token_list_test, "boost.http_proto.token_list");

} // http_proto
} // boost
