//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/rfc/list_bnf.hpp>

#include <boost/http_proto/rfc/token_bnf.hpp>
#include <boost/url/bnf/parse.hpp>

#include "test_suite.hpp"

#include <initializer_list>

namespace boost {
namespace http_proto {

class list_bnf_test
{
public:
    template<
        std::size_t N = 0,
        std::size_t M =
            std::size_t(-1)>
    void
    bad(string_view s)
    {
        error_code ec;
        list_bnf<token_bnf, N, M> t;
        auto const success =
            urls::bnf::parse_string(s, ec, t);
        if(! BOOST_TEST(! success))
            return;
        BOOST_TEST(ec.failed());
    }

    template<
        std::size_t N = 0,
        std::size_t M =
            std::size_t(-1)>
    void
    good(
        string_view s,
        std::initializer_list<
            string_view> init)
    {
        error_code ec;
        list_bnf<token_bnf, N, M> t;
        auto const success =
            urls::bnf::parse_string(s, ec, t);
        if(! BOOST_TEST(success))
            return;
        if(ec.failed())
            return;
        if(! BOOST_TEST(t.size() == init.size()))
            return;
        auto it = t.begin();
        for(std::size_t i = 0;
            i < t.size(); ++i)
            BOOST_TEST(*it++ == init.begin()[i]);
    }

    void
    testSpecial()
    {
        list_bnf<token_bnf> t("x,y");
        BOOST_TEST(t.size() == 2);
    }

    void
    testParse()
    {
        good("", {});
        good("x", {"x"});
        good("x,y", {"x","y"});

        bad(" ");
        bad("\t");
        bad(" \t");
        bad("     ");

        bad( ",");
        bad( ", ");
        bad( ", ,");
        bad( ",,,");
        bad( "1, ");
        good("", {});
        good("1", {"1"});
        good(",1", {"1"});
        good("1,", {"1"});
        good(", 1", {"1"});
        good("1 ,", {"1"});
        good("1,2", {"1", "2"});
        good("1,2", {"1", "2"});
        good("1,2,3", {"1", "2", "3"});
        good(", 1,\t2, 3", {"1", "2", "3"});
    }

    void
    run()
    {
        testSpecial();
        testParse();
    }
};

TEST_SUITE(
    list_bnf_test,
    "boost.http_proto.list_bnf");

} // http_proto
} // boost
