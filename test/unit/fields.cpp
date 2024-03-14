//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields.hpp>

#include <boost/http_proto/field.hpp>         // for field
#include <boost/http_proto/fields_view.hpp>   // for fields_view
#include <boost/core/detail/string_view.hpp>  // for string_view

#include "test_helpers.hpp"                   // for test_fields
#include "test_suite.hpp"                     // for test_with_impl, BOOST_TEST_EQ, BOOST_TEST_NE, BOOST_TEST, BOOST...

#include <stdexcept>                          // for length_error
#include <utility>                            // for move

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
        auto check = [](
            fields& f,
            std::size_t initial_size,
            std::size_t max_capacity)
        {
            auto const old = f.buffer().data();
            f.append(field::host, "www.google.com");
            f.append(field::connection, "close");
            f.insert(
                f.find(field::host),
                field::content_length, "1234");

            BOOST_TEST_EQ(
                f.buffer().data(), old);
            BOOST_TEST_EQ(
                f.capacity_in_bytes(), initial_size);
            BOOST_TEST_EQ(
                f.max_capacity_in_bytes(), max_capacity);
            BOOST_TEST_THROWS(
                f.reserve_bytes(max_capacity + 1),
                std::length_error);
        };

        {
            std::size_t initial_size = 4096;
            std::size_t max_capacity = initial_size;

            fields f(initial_size);
            check(f, initial_size, max_capacity);
        }

        {
            std::size_t initial_size = 4096;
            std::size_t max_capacity = 8192;

            fields f(initial_size, max_capacity);
            check(f, initial_size, max_capacity);
        }

        {
            std::size_t initial_size = 4096;
            std::size_t max_capacity = initial_size;

            fields f(initial_size);
            check(f, initial_size, max_capacity);

            fields f2(2 * initial_size);
            check(f2, 2 * initial_size, 2 * initial_size);

            f = f2;
            check(f, initial_size, 2 * initial_size);
        }

        {
            std::size_t initial_size = 4096;
            std::size_t max_capacity = initial_size;

            fields f(initial_size);
            check(f, initial_size, max_capacity);

            fields f2(2 * initial_size);
            check(f2, 2 * initial_size, 2 * initial_size);

            f = std::move(f2);
            check(f, 2 * initial_size, 2 * initial_size);
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
