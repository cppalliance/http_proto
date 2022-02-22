//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/transfer_encoding.hpp>

#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"
#include "test_rule.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(is_element<transfer_coding>::value);
BOOST_STATIC_ASSERT(is_list<transfer_encoding>::value);

class transfer_encoding_test
{
public:
    static
    void
    good(
        string_view s,
        string_view match)
    {
        range<transfer_encoding> r(s);
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
        using T = transfer_encoding;

        test::bad<T>("");
        test::bad<T>("@");
        test::bad<T>(" ");
        test::bad<T>(" x");
        test::bad<T>("x ");
        test::bad<T>("d;b=3 ");
        test::bad<T>(" d;b=3");

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

TEST_SUITE(transfer_encoding_test, "boost.http_proto.transfer_encoding");

} // bnf
} // http_proto
} // boost
