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
    check(
        method m,
        string_view t,
        int v,
        string_view const s)
    {
        auto const f =
            [&](request_parser const& p)
        {
            auto const req = p.get();
            BOOST_TEST(req.method() == m);
            BOOST_TEST(req.method_str() ==
                to_string(m));
            BOOST_TEST(req.target() == t);
            BOOST_TEST(req.version() == v);
        };

        // single buffer
        {
            error_code ec;
            request_parser p;
            while(! p.is_done())
            {
                auto const b = p.prepare();
                auto const n = (std::min)(
                    b.second, s.size());
                BOOST_TEST(n == s.size());
                std::memcpy(
                    b.first, s.data(), n);
                auto const n1 = p.commit(n, ec);
                BOOST_TEST(! ec);
                if(ec)
                    break;
            }
            if(! ec)
                f(p);
        }

        // two buffers
        for(std::size_t i = 1;
            i < s.size(); ++i)
        {
            error_code ec;
            request_parser p;
            while(! p.is_done())
            {
                // first buffer
                auto b = p.prepare();
                auto n = (std::min)(
                    b.second, i);
                BOOST_TEST(n == i);
                std::memcpy(
                    b.first, s.data(), n);
                auto n1 = p.commit(n, ec);
                BOOST_TEST(! ec);
                if(ec)
                    break;

                // second buffer
                b = p.prepare();
                n = (std::min)(
                    b.second, s.size());
                BOOST_TEST(n == s.size());
                std::memcpy(
                    b.first, s.data() + i, n);
                n1 = p.commit(n, ec);
                BOOST_TEST(! ec);
                if(ec)
                    break;
            }
            if(! ec)
                f(p);
        }
    }

    static
    void
    test()
    {
        check(method::get, "/", 11,
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

