//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/transfer_param_list.hpp>

#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"
#include "test_rule.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(
    is_list<transfer_param_list>::value);

class transfer_param_list_test
{
public:
    template<class T>
    static
    void
    good(
        string_view s,
        string_view match)
    {
        range<T> r(s);
        BOOST_TEST_NO_THROW(r.validate());
        if(! BOOST_TEST(r.is_valid()))
            return;
        std::stringstream ss;
        for(auto it = r.begin(),
            end = r.end();
            it != end;)
        {
            auto v = *it++;
            ss << ";" <<
                v.name << "=" <<
                v.value;
        }
        BOOST_TEST(ss.str() == match);
    }
    void
    run()
    {
        using namespace test;
        using T = transfer_param_list;

        test::bad<T>(" ");
        test::bad<T>(" ;");
        test::bad<T>("; ");
        test::bad<T>(";");
        test::bad<T>(";a");
        test::bad<T>(";a=");
        test::bad<T>(";a=b ");
        test::bad<T>(";a=b;");
        test::bad<T>(";a=\"");

        good<T>("", "");
        good<T>(";a=b", ";a=b");
        good<T>(" ;a=b", ";a=b");
        good<T>("; a=b", ";a=b");
        good<T>(";a =b", ";a=b");
        good<T>(";a= b", ";a=b");
        good<T>(";a=b;c=d", ";a=b;c=d");
        good<T>(";a=\"x\"", ";a=\"x\"");

    }
};

TEST_SUITE(transfer_param_list_test,
    "boost.http_proto.transfer_param_list");

} // bnf
} // http_proto
} // boost
