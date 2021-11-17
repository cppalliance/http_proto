//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/bnf/list.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/bnf/algorithm.hpp>
#include <boost/http_proto/bnf/ctype.hpp>

#include <boost/url/bnf/charset.hpp>

#include "test_bnf.hpp"

namespace boost {
namespace http_proto {
namespace bnf {

class list_test
{
public:
    /*
        test-element = 1*DIGIT
    */
    struct test_element
    {
        using value_type = string_view;

        string_view v_;

        string_view value() const noexcept
        {
            return v_;
        }

        char const*
        parse(
            char const* start,
            char const* end,
            error_code& ec)
        {
            if(start == end)
            {
                ec = error::need_more;
                return start;
            }
            auto const ds = urls::bnf::digit_chars;
            auto it = start;
            if(! ds(*it))
            {
                ec = error::syntax;
                return it;
            }
            it = ds.find_if_not(it + 1, end);
            return it;
        }
    };

    void
    run()
    {
        {
            using T = list_of_zero_or_more<
                test_element>;
            test::bad<T>( ",");
            test::bad<T>( ", ");
            test::bad<T>( ", ,");
            test::bad<T>( ",,,");
            test::bad<T>( "1, ");
            test::good<T>("");
            test::good<T>("1");
            test::good<T>(",1");
            test::good<T>("1,");
            test::good<T>(", 1");
            test::good<T>("1 ,");
            test::good<T>("1,2");
            test::good<T>("1,2");
            test::good<T>("1,2,3");
            test::good<T>(", 1,\t2, 3");
        }
        {
            using T = list_of_one_or_more<
                test_element>;
            test::bad<T>( "");
            test::bad<T>( ",");
            test::bad<T>( ", ");
            test::bad<T>( ", ,");
            test::bad<T>( ",,,");
            test::good<T>("1");
            test::good<T>("1,2");
        }
        {
            using T = list<test_element, 2, 3>;
            test::bad<T>( "");
            test::bad<T>( ",");
            test::bad<T>( ", ");
            test::bad<T>( ", ,");
            test::bad<T>( ",,,");
            test::bad<T>( "1");
            test::bad<T>( ",1");
            test::bad<T>( "1,");
            test::bad<T>( ", 1");
            test::bad<T>( "1 ,");
            test::bad<T>( "1, ");
            test::bad<T>( "1,2,3,4");
            test::good<T>("1,2");
            test::good<T>("1,2");
            test::good<T>("1,2,3");
            test::good<T>(", 1,\t2, 3");
        }
    }
};

TEST_SUITE(list_test, "boost.http_proto.list");

} // bnf
} // http_proto
} // boost
