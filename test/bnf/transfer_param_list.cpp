//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/transfer_param_list.hpp>

#include "test_suite.hpp"
#include "test_bnf.hpp"

namespace boost {
namespace http_proto {

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
        T r(s);
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
                v.first << "=" <<
                v.second;
        }
        BOOST_TEST(ss.str() == match);
    }
    void
    run()
    {
        using namespace test;
        using T = transfer_param_list;

        bad<T>(" ");
        bad<T>(" ;");
        bad<T>("; ");
        bad<T>(";");
        bad<T>(";a");
        bad<T>(";a=");
        bad<T>(";a=b ");
        bad<T>(";a=b;");

        good<T>("", "");
        good<T>(";a=b", ";a=b");
        good<T>(" ;a=b", ";a=b");
        good<T>("; a=b", ";a=b");
        good<T>(";a =b", ";a=b");
        good<T>(";a= b", ";a=b");
        good<T>(";a=b;c=d", ";a=b;c=d");
    }
};

TEST_SUITE(transfer_param_list_test, "boost.http_proto.transfer_param_list");

} // http_proto
} // boost
