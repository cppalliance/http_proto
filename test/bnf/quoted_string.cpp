//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/quoted_string.hpp>

#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/assert.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(
    is_element<quoted_string>::value);

class quoted_string_test
{
public:
    template<class T>
    void
    valid(string_view s)
    {
        BOOST_TEST_NO_THROW(
            validate<T>(s));
        BOOST_TEST(is_valid<T>(s));
    }

    template<class T>
    void
    invalid(string_view s)
    {
        BOOST_TEST_THROWS(
            validate<T>(s),
            std::exception);
        BOOST_TEST(! is_valid<T>(s));
    }

    void
    run()
    {
        using T = quoted_string;
        invalid<T>("");
        invalid<T>(" ");
        invalid<T>("\t");
        invalid<T>("x");
        invalid<T>("\"");
        invalid<T>(" \"\"");
          valid<T>("\"" "\"");
          valid<T>("\"" "x" "\"");
          valid<T>("\"" "\t" "\"");
          valid<T>("\"" " \x80" "\"");
    }
};

TEST_SUITE(quoted_string_test, "boost.http_proto.quoted_string");

} // bnf
} // http_proto
} // boost
