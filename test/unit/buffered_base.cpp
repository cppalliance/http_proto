//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/buffered_base.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct buffered_base_test
{
    struct max_base : buffered_base
    {
        explicit
        max_base(
            std::size_t max_size) noexcept
            : max_size_(max_size)
        {
        }

        void
        on_init(allocator& a) override
        {
            BOOST_TEST(
                a.max_size() == max_size_);
        }

    private:
        std::size_t max_size_;
    };

    struct alloc_base : buffered_base
    {
        explicit
        alloc_base(
            std::size_t alloc) noexcept
            : alloc_(alloc)
        {
        }

        void
        on_init(allocator& a) override
        {
            a.allocate(alloc_);
        }

    private:
        std::size_t alloc_;
    };

    void
    testMembers()
    {
        // init(allocator&)

        {
            buffered_base b;
            buffered_base::allocator a;
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(), 0);
        }

        {
            max_base b(0);
            buffered_base::allocator a;
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(), 0);
        }

        {
            char temp[15];
            max_base b(sizeof(temp));
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(
                a.max_size(), sizeof(temp));
        }

        {
            // downward
            char temp[15];
            max_base b(sizeof(temp));
            buffered_base::allocator a(
                temp, sizeof(temp), true);
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(
                a.max_size(), sizeof(temp));
        }

        // init(allocator&, std::size_t)

        {
            buffered_base b;
            buffered_base::allocator a;
            BOOST_TEST_NO_THROW(b.init(a, 0));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(), 0);
        }

        {
            max_base b(0);
            buffered_base::allocator a;
            BOOST_TEST_NO_THROW(b.init(a, 0));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(), 0);
        }

        {
            char temp[15];
            max_base b(0);
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_NO_THROW(b.init(a, 0));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(
                a.max_size(), sizeof(temp));
        }

        {
            char temp[15];
            max_base b(0);
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_THROWS(
                b.init(a, 20),
                std::invalid_argument);
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(
                a.max_size(), sizeof(temp));
        }
    }

    void
    testAllocate()
    {
        // allocate(0)

        {
            alloc_base b(0);
            buffered_base::allocator a;
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(), 0);
        }

        {
            char temp[15];
            alloc_base b(0);
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(
                a.max_size(), sizeof(temp));
        }

        {
            // downward
            char temp[15];
            alloc_base b(0);
            buffered_base::allocator a(
                temp, sizeof(temp), true);
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(
                a.max_size(), sizeof(temp));
        }

        // allocate(std::size_t)

        {
            alloc_base b(1);
            buffered_base::allocator a;
            BOOST_TEST_THROWS(
                b.init(a),
                std::invalid_argument);
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(), 0);
        }

        {
            char temp[15];
            alloc_base b(1);
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 1);
            BOOST_TEST_EQ(a.max_size(),
                sizeof(temp) - 1);
        }

        {
            // downward
            char temp[15];
            alloc_base b(1);
            buffered_base::allocator a(
                temp, sizeof(temp), true);
            BOOST_TEST_NO_THROW(b.init(a));
            BOOST_TEST_EQ(a.size_used(), 1);
            BOOST_TEST_EQ(a.max_size(),
                sizeof(temp) - 1);
        }

        {
            char temp[15];
            alloc_base b(5);
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_NO_THROW(b.init(a, 10));
            BOOST_TEST_EQ(a.size_used(), 5);
            BOOST_TEST_EQ(a.max_size(),
                sizeof(temp) - 5);
        }

        {
            // downward
            char temp[15];
            alloc_base b(5);
            buffered_base::allocator a(
                temp, sizeof(temp), true);
            BOOST_TEST_NO_THROW(b.init(a, 10));
            BOOST_TEST_EQ(a.size_used(), 5);
            BOOST_TEST_EQ(a.max_size(),
                sizeof(temp) - 5);
        }

        {
            char temp[15];
            alloc_base b(10);
            buffered_base::allocator a(
                temp, sizeof(temp), false);
            BOOST_TEST_THROWS(
                b.init(a, 5),
                std::invalid_argument);
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(),
                sizeof(temp));
        }

        {
            // downward
            char temp[15];
            alloc_base b(10);
            buffered_base::allocator a(
                temp, sizeof(temp), true);
            BOOST_TEST_THROWS(
                b.init(a, 5),
                std::invalid_argument);
            BOOST_TEST_EQ(a.size_used(), 0);
            BOOST_TEST_EQ(a.max_size(),
                sizeof(temp));
        }
    }

    void
    run()
    {
        testMembers();
        testAllocate();
    }
};

TEST_SUITE(
    buffered_base_test,
    "boost.http_proto.buffered_base");

} // http_proto
} // boost
