//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_parser.hpp>
#include <algorithm>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class request_parser_test
{
public:
    static
    void
    check(string_view s)
    {
        request_parser p;
        while(! s.empty())
        {
            auto const b = p.prepare();
            auto const n = (std::min)(
                b.second, s.size());
            std::memcpy(
                b.first, s.data(), n);
            s.remove_prefix(n);
            error_code ec;
            auto const n1 = p.commit(n, ec);
            BOOST_TEST(! ec);
            if(ec)
                break;
        }
    }

    static
    void
    test()
    {
        check(
            "GET / HTTP/1.1\r\n"
            "Connection: close\r\n"
            "\r\n");
    }

    void
    run()
    {
        test();
    }
};

TEST_SUITE(request_parser_test, "boost.http_proto.request_parser");

} // http_proto
} // boost

