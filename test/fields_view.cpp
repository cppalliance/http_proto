//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/field.hpp>

#include "test_suite.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct fields_view_test
{
    fields_view
    construct(
        string_view s)
    {
        return fields_view(s);
    }

    fields_view
    construct(
        string_view s0,
        std::string& s)
    {
        auto f = construct(s0);
        auto A = alignof(
            detail::fields_table_entry);
        auto n = A * (
            (s0.size() + A - 1) / A) +
            detail::fields_table_size(
                f.size());
        s.resize(n);
        std::memcpy(&s[0],
            s0.data(), s0.size());
        std::size_t i = 0;
        detail::fields_table t(
            &s[0] + s.size());
        for(auto const& v : f)
            detail::write(
                t,
                s0.data(),
                i++,
                v.name,
                v.value,
                v.id);
        fields_view::ctor_params init;
        init.base = s.data();
        init.start_len = 0;
        init.end_len = s0.size();
        init.count = f.size();
        init.table = &s[0] + s.size();
        return fields_view(init);        
    }

    void
    testOneView(fields_view f)
    {
        // size
        BOOST_TEST(f.size() == 10);

        // exists
        BOOST_TEST(! f.exists("a"));
        BOOST_TEST(f.exists("x"));
        BOOST_TEST(f.exists("X"));
        BOOST_TEST(f.exists("user-agent"));
        BOOST_TEST(! f.exists(
            field::range));
        BOOST_TEST(f.exists(
            field::user_agent));

        // count
        BOOST_TEST(f.count("x") == 3);
        BOOST_TEST(f.count("y") == 1);
        BOOST_TEST(f.count("q") == 0);
        BOOST_TEST(f.count(
            field::range) == 0);
        BOOST_TEST(f.count(
            field::user_agent) == 1);

        // at
        BOOST_TEST(f.at("x") == "1");
        BOOST_TEST(f.at(
            field::user_agent) == "boost");
        BOOST_TEST_THROWS(f.at("q"),
            std::invalid_argument);
        BOOST_TEST_THROWS(f.at(
            field::range),
            std::invalid_argument);

        // value_or
        BOOST_TEST(f.value_or(
            field::content_length, "69") == "42");
        BOOST_TEST(f.value_or(
            field::user_agent, "42") == "boost");
        BOOST_TEST(f.value_or(
            field::range, "0-1") == "0-1");
        BOOST_TEST(f.value_or("x", "*") == "1");
        BOOST_TEST(f.value_or("q", "*") == "*");

        // find
        {
            auto it = f.find("z");
            BOOST_TEST(it->value == "4");
            it = f.find("x");
            BOOST_TEST(it->value == "1");
            it = f.find(field::range);
            BOOST_TEST(it == f.end());
            it = f.find(field::user_agent);
            BOOST_TEST(it->id ==
                field::user_agent);

            it = f.find(++(++f.begin()), "x");
            BOOST_TEST(it->value == "3");

            it = f.find(++f.begin(),
                field::user_agent);
            BOOST_TEST(it->value == "boost");
        }

        // find_all
        // make_list
        BOOST_TEST(
            make_list(f.find_all("x")) ==
            "1,3,5");
        BOOST_TEST(
            make_list(f.find_all("y")) ==
            "2");
        BOOST_TEST(
            make_list(f.find_all("q")) == "");
        BOOST_TEST(
            make_list(f.find_all(
                field::user_agent)) == "boost");
        BOOST_TEST(
            make_list(f.find_all(
                field::set_cookie)) == "a,b");

        // iterator
        BOOST_TEST(
            fields_view::iterator() ==
            fields_view::iterator());

        auto it = f.begin();
        BOOST_TEST(it == f.begin());
        BOOST_TEST(it == f.begin()++);
        BOOST_TEST(it != f.end());
        BOOST_TEST(it->id ==
            field::content_length);
        BOOST_TEST(it->name ==
            "Content-Length");
        BOOST_TEST(it->value == "42");

        ++it;
        ++it;
        ++it;
        BOOST_TEST(it != f.begin());
        BOOST_TEST(it == ++(++(++f.begin())));
        BOOST_TEST(it != f.end());
        BOOST_TEST(it->id ==
            field::set_cookie);
        BOOST_TEST(it->name ==
            "Set-Cookie");
        BOOST_TEST(it->value == "a");

        ++it;
        BOOST_TEST(it != f.begin());
        BOOST_TEST(it != f.end());
    }

    void
    testEmptyView(fields_view f)
    {
        // empty range
        BOOST_TEST(f.size() == 0);
        BOOST_TEST(f.begin() == f.end());
    }

    void
    testViews()
    {
        string_view const cs =
            "Content-Length: 42\r\n"
            "x: 1\r\n"
            "y: 2\r\n"
            "Set-Cookie: a\r\n"
            "x: 3\r\n"
            "z: 4\r\n"
            "Set-Cookie: b\r\n"
            "x: 5\r\n"
            "p: 6\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        {
            auto f = construct(cs);
            testOneView(f);
        }

        {
            std::string s;
            auto f = construct(cs, s);
            testOneView(f);
        }

        {
            // invalid input
            BOOST_TEST_THROWS(
                construct(""),
                system_error);
        }
    }

    void
    testSubrange()
    {
        using S =
            fields_view::subrange;

        auto f = construct(
            "x: 1\r\n"
            "y: 2\r\n"
            "x: 3\r\n"
            "z: 4\r\n"
            "x: 5\r\n"
            "p: 6\r\n"
            "User-Agent: boost\r\n"
            "\r\n");

        // subrange
        {
            S sr;
            BOOST_TEST(
                sr.begin() == sr.end());
        }

        // subrange(subrange const&)
        {
            S sr;
            S sr2(sr);
            BOOST_TEST(
                sr.begin() == sr.end());
        }

        // operator=(subrange const&)
        {
            S sr = f.find_all("x");
            S sr2;
            sr2 = sr;
            BOOST_TEST(
                sr2.begin() == sr.begin());
        }

        // iterators
        BOOST_TEST(
            S::iterator() ==
            S::iterator());

        {
            S sr = f.find_all("x");
            auto it = sr.begin();
            BOOST_TEST(it == sr.begin());
            BOOST_TEST(it == sr.begin()++);
            BOOST_TEST(it != sr.end());
            BOOST_TEST(
                it->id == field::unknown);
            BOOST_TEST(it->name == "x");
            BOOST_TEST(it->value == "1");

            ++it;
            BOOST_TEST(it != sr.begin());
            BOOST_TEST(it == ++sr.begin());
            BOOST_TEST(it != sr.end());
            BOOST_TEST(it->name == "x");
            BOOST_TEST(it->value == "3");

            ++it;
            BOOST_TEST(it != sr.begin());
            BOOST_TEST(it != sr.end());
            BOOST_TEST(it->name == "x");
            BOOST_TEST(it->value == "5");

            ++it;
            BOOST_TEST(it == sr.end());
        }

        {
            S sr = f.find_all(
                field::user_agent);
            auto it = sr.begin();
            BOOST_TEST(it->value == "boost");
        }
    }

    void
    run()
    {
        testViews();
        testSubrange();
    }
};

TEST_SUITE(
    fields_view_test,
    "boost.http_proto.fields_view");

} // http_proto
} // boost
