//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/transfer_encoding_list.hpp>

#include "test_suite.hpp"
#include "test_bnf.hpp"

namespace boost {
namespace http_proto {

class transfer_encoding_list_test
{
public:
    static
    void
    good(
        string_view s,
        string_view match)
    {
        transfer_encoding_list r(s);
        BOOST_TEST_NO_THROW(r.validate());
        if(! BOOST_TEST(r.is_valid()))
            return;
        std::stringstream ss;
        for(auto it = r.begin(),
            end = r.end();
            it != end;)
        {
            auto const v = *it++;
            ss << v.name;
            for(auto p : v.params)
                ss << ";" << p.name <<
                    "=" << p.value;
            if(it != end)
                ss << ',';
        }
        BOOST_TEST(ss.str() == match);
    }

    void
    run()
    {
        using namespace test;
        using T = transfer_encoding_list;

        bad<T>("");
        bad<T>("@");
        bad<T>(" ");
        bad<T>(" x");
        bad<T>("x ");
        bad<T>("d;b=3 ");
        bad<T>(" d;b=3");

        good("x","x");
        good(",x","x");
        good("x,","x");
        good("chunked","chunked");
        good("d;b=3","d;b=3");
        good("d ;b=3","d;b=3");
        good("d; b=3","d;b=3");
        good("d;b =3","d;b=3");
        good("d;b= 3","d;b=3");
        good("a,b,c","a,b,c");
        good("a ,b,c","a,b,c");
        good("a, b,c","a,b,c");
        good("a,b ,c","a,b,c");
        good("a,b, c","a,b,c");
    }
};

TEST_SUITE(transfer_encoding_list_test, "boost.http_proto.transfer_encoding_list");

} // http_proto
} // boost
