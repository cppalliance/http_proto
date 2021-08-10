//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/trivial_optional.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/none.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class optional_test
{
public:
    void
    run()
    {
        // default-ctor
        // ~trivial_optional
        {
            trivial_optional<int> v;
            BOOST_TEST(! v);
            BOOST_TEST(! v.has_value());
        }

        // trivial_optional(none_t)
        {
            trivial_optional<int> v = boost::none;
            BOOST_TEST(! v);
            BOOST_TEST(! v.has_value());
        }

        // copy-ctor
        // trivial_optional(T)
        {
            trivial_optional<int> v1(42);
            trivial_optional<int> v2(v1);
            auto const& cv2 = v2;
            BOOST_TEST(v2.value_or(0) == 42);
            BOOST_TEST(cv2.value_or(0) == 42);
        }

        // operator=
        // operator*
        // operator->
        // value() throwing
        {
            trivial_optional<string_view> v("test");
            auto const& cv = v;
            v = "xyz";
            BOOST_TEST(*v == "xyz");
            BOOST_TEST(v->size() == 3);
            BOOST_TEST(cv->size() == 3);
            v = boost::none;
            BOOST_TEST(! v.has_value());
            BOOST_TEST(! cv.has_value());
            BOOST_TEST_THROWS(v.value(),
                std::exception);
            BOOST_TEST_THROWS(cv.value(),
                std::exception);
        }

        // operator==
        // operator!=
        // operator=
        // value_or
        {
            trivial_optional<string_view> v1("test");
            trivial_optional<string_view> v2("xyz");
            BOOST_TEST(v1 != v2);
            v2 = boost::none;
            BOOST_TEST(v1 != v2);
            v1 = v2;
            BOOST_TEST(v1 == v2);
            BOOST_TEST(! v1.has_value());
            BOOST_TEST(v2.value_or("42") == "42");
        }

        // emplace
        {
            trivial_optional<int> v;
            BOOST_TEST(! v);
            v.emplace(42);
            BOOST_TEST(v);
            BOOST_TEST(*v == 42);
            v.emplace();
            BOOST_TEST(v);
            BOOST_TEST(*v == 0);
        }
    }
};

TEST_SUITE(optional_test, "boost.http_proto.optional");

} // http_proto
} // boost
