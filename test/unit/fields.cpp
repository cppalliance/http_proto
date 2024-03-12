//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields.hpp>

#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <initializer_list>

#include "test_helpers.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct fields_test
{
    void
    modify(
        core::string_view before,
        void (*pf)(fields&),
        core::string_view after)
    {
        fields f0(before);
        fields f1(after);
        fields f(f0);
        (*pf)(f);
        BOOST_TEST_EQ(f.buffer(),
            f1.buffer());
    }

    //--------------------------------------------

    void
    testSpecial()
    {
        core::string_view const cs1 =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        core::string_view const cs2 =
            "x: 1\r\n"
            "y: 2\r\n"
            "z: 3\r\n"
            "\r\n";

        // ~fields()
        // fields()
        {
            fields f;
            BOOST_TEST_EQ(
                f.buffer(), "\r\n");
            BOOST_TEST_EQ(
                f.buffer().data(),
                fields().buffer().data());
        }

        // fields(fields&&)
        {
            {
                fields f1(cs1);
                fields f2(std::move(f1));
                test_fields(f1, "\r\n");
                test_fields(f2, cs1);
            }
            {
                fields f1;
                fields f2(std::move(f1));
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
        }

        // fields(fields const&)
        {
            {
                fields f1(cs1);
                fields f2(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                fields f1;
                fields f2(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
        }

        // fields(fields_view const&)
        {
            {
                fields f1(cs1);
                fields f2(static_cast<
                    fields_view>(f1));

                BOOST_TEST_EQ(
                    f2.buffer(), cs1);
                BOOST_TEST_NE(
                    f2.buffer().data(),
                    cs1.data());
                test_fields(f2, cs1);
            }

            // default buffer
            {
                fields_view fv;
                fields f(fv);
                BOOST_TEST_EQ(
                    f.buffer(), "\r\n");
                BOOST_TEST_EQ(
                    f.buffer().data(),
                    fv.buffer().data());
            }
        }

        // operator=(fields&&)
        {
            {
                fields f1(cs1);
                fields f2;
                f2 = std::move(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, cs1);
            }
            {
                fields f1(cs1);
                fields f2(cs2);
                f2 = std::move(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, cs1);
            }
            {
                fields f1;
                fields f2(cs1);
                f2 = std::move(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
        }

        // operator=(fields const&)
        {
            {
                fields f1(cs1);
                fields f2;
                f2 = f1;
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                fields f1(cs1);
                fields f2(
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "z: 3\r\n"
                    "\r\n");
                f2 = f1;
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                fields f1;
                fields f2(cs1);
                f2 = f1;
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
        }

        // operator=(fields_view)
        {
            {
                fields f1(cs1);
                fields f2;
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                fields f1(cs1);
                fields f2(
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "z: 3\r\n"
                    "\r\n");
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                fields_view f1;
                fields f2(cs1);
                f2 = f1;
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }

            // existing capacity
            {
                fields f1(cs1);
                fields f2;
                f2.reserve_bytes(
                    2 * cs1.size() + 128);
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f2, cs1);
                BOOST_TEST(
                    f1.buffer().data() !=
                    f2.buffer().data());
            }
        }
    }

    void
    testObservers()
    {
        core::string_view const cs =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        // fields_view_base::string()
        {
            fields f1(cs);
            BOOST_TEST_EQ(f1.buffer(), cs);
        }

        // fields_base::capacity_in_bytes()
        {
            {
                fields f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0U);
            }
            {
                fields f;
                f.reserve_bytes(100);
                BOOST_TEST_GE(
                    f.capacity_in_bytes(), 100U);
            }
            {
                fields f;
                f.reserve_bytes(100);
                f.shrink_to_fit();
                BOOST_TEST_GT(
                    f.capacity_in_bytes(), 0U);
            }
        }
    }

    void
    testInitialSize()
    {
        {
            std::size_t initial_size = 4096;
            fields f(initial_size);
            BOOST_TEST_EQ(
                f.capacity_in_bytes(), 4096);
            BOOST_TEST_EQ(
                f.max_capacity_in_bytes(), f.capacity_in_bytes());
        }
    }

    void
    run()
    {
        testSpecial();
        testObservers();
        testInitialSize();
    }
};

TEST_SUITE(
    fields_test,
    "boost.http_proto.fields");

} // http_proto
} // boost
