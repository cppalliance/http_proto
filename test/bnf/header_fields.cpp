//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/header_fields.hpp>

#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"
#include "test_bnf.hpp"

#include <string>

namespace boost {
namespace http_proto {
namespace bnf {

BOOST_STATIC_ASSERT(
    is_list<header_fields>::value);

class header_fields_test
{
public:
    void
    testReplaceObsFold()
    {
        auto const check =
            [](string_view sv0, string_view sv1)
        {
            std::string s(
                sv0.data(), sv0.size());
            replace_obs_fold(
                &s[0], s.data() + s.size());
            BOOST_TEST(sv1 == s);
        };

        check("", "");
        check(" ", " ");
        check("\t", "\t");
        check("\r", "\r");
        check("\r\n", "\r\n");
        check(" \r\n", " \r\n");
        check("\r\n\r", "\r\n\r");
        check(" \r\n.", " \r\n.");
        check(" \r\n\t", "   \t");
        check("\r\n\r\n", "\r\n\r\n");
        check(".\r\n .\r\n", ".   .\r\n");
        check(" \r\n \r", "    \r");
        check(" \r\n \r ", "    \r ");
    }

    void
    run()
    {
        testReplaceObsFold();
    }
};

TEST_SUITE(header_fields_test, "boost.http_proto.header_fields");

} // bnf
} // http_proto
} // boost
