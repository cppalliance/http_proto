//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/static_fields.hpp>

#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/core/detail/string_view.hpp>

#include "test_helpers.hpp"
#include "test_suite.hpp"

#include <stdexcept>
#include <utility>

namespace boost {
namespace http_proto {

struct static_fields_test
{
    template<std::size_t Capacity>
    void
    modify(
        core::string_view before,
        void (*pf)(static_fields<256>&),
        core::string_view after)
    {
        static_fields<256> f0(before);
        static_fields<256> f1(after);
        static_fields<256> f(f0);
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

        // ~fields()
        // static_fields()
        {
            static_fields<256> f;
            BOOST_TEST_EQ(
                f.buffer(), "\r\n");
            BOOST_TEST_EQ(
                f.buffer().data(),
                static_fields<256>().buffer().data());
        }

        // static_fields(static_fields<256> const&)
        {
            {
                static_fields<256> f1(cs1);
                static_fields<256> f2(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                static_fields<256> f1;
                static_fields<256> f2(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
        }

        // static_fields(fields_view const&)
        {
            {
                static_fields<256> f1(cs1);
                static_fields<256> f2(static_cast<
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
                static_fields<256> f(fv);
                BOOST_TEST_EQ(
                    f.buffer(), "\r\n");
                BOOST_TEST_EQ(
                    f.buffer().data(),
                    fv.buffer().data());
            }
        }

        // operator=(static_fields<256> const&)
        {
            {
                static_fields<256> f1(cs1);
                static_fields<256> f2;
                f2 = f1;
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                static_fields<256> f1(cs1);
                static_fields<256> f2(
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
                static_fields<256> f1;
                static_fields<256> f2(cs1);
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
                static_fields<256> f1(cs1);
                static_fields<256> f2;
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.buffer().data(),
                    f2.buffer().data());
            }
            {
                static_fields<256> f1(cs1);
                static_fields<256> f2(
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
                static_fields<256> f2(cs1);
                f2 = f1;
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.buffer().data(),
                    f2.buffer().data());
            }

            // existing capacity
            {
                static_fields<256> f1(cs1);
                static_fields<512> f2;
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
            static_fields<256> f1(cs);
            BOOST_TEST_EQ(f1.buffer(), cs);
        }

        // fields_base::capacity_in_bytes()
        {
            {
                static_fields<256> f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 256U);
            }
            {
                static_fields<256> f;
                f.reserve_bytes(100);
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 256U);
            }
            {
                static_fields<256> f;
                f.reserve_bytes(100);
                f.shrink_to_fit();
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 256U);
            }
        }
    }

    void
    testInitialSize()
    {
        {
            static_fields<16> f{};
            BOOST_TEST_THROWS(
                f.append(field::host, "www.google.com"), std::length_error);
            BOOST_TEST_EQ(
                f.max_capacity_in_bytes(), 16);
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
    static_fields_test,
    "boost.http_proto.static_fields");

} // http_proto
} // boost
