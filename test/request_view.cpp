//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/http_proto/rfc/request_line_rule.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/url/grammar/parse.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class request_view_test
{
public:
    struct test_view : request_view
    {
        explicit
        test_view(string_view s)
            : request_view(
            [&s]
            {
                error_code ec;
                auto it = s.data();
                auto const end =
                    it + s.size();
                request_line_rule t0;
                if(! grammar::parse(
                    it, end, ec, t0))
                    detail::throw_system_error(
                        ec, BOOST_CURRENT_LOCATION);

                ctor_params init;
                init.base = s.data();
                init.start_len = 0;
                init.end_len = s.size();
                init.count = 0;
                init.table = nullptr;
                init.method_len = t0.ms.size();
                init.target_len = t0.t.size();
                init.method = t0.m;
                init.version = t0.v;
                field_rule t1;
                for(;;)
                {
                    if(grammar::parse(
                        it, end, ec, t1))
                    {
                        ++init.count;
                        continue;
                    }
                    if(ec == grammar::error::end)
                        break;
                    detail::throw_system_error(ec,
                        BOOST_CURRENT_LOCATION);
                }
                return init;
            }())
        {
        }
    };

    void
    run()
    {
        string_view s =
            "GET / HTTP/1.1\r\n"
            "Content-Length: 42\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        // default ctor
        {
            request_view req;
            BOOST_TEST(req.size() == 0);
        }

        // copy ctor
        {
            test_view r1(s);
            BOOST_TEST(
                r1.buffer().data() ==
                    s.data());
            request_view r2(r1);
            BOOST_TEST(r2.size() == 2);;
            BOOST_TEST(
                r2.buffer().data() ==
                    s.data());
        }

        // copy assign
        {
            test_view r1(s);
            request_view r2;
            BOOST_TEST(r2.buffer().empty());
            r2 = r1;
            BOOST_TEST(
                r2.buffer().data() ==
                    s.data());
        }
    }
};

TEST_SUITE(
    request_view_test,
    "boost.http_proto.request_view");

} // http_proto
} // boost

