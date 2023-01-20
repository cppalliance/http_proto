//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/buffer.hpp>

#include <boost/http_proto/string_view.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct asio_mutable_buffer
{
    std::size_t size() const noexcept { return 0; }
    void* data() const noexcept { return nullptr; }
};

struct asio_const_buffer
{
    std::size_t size() const noexcept { return 0; }
    void const* data() const noexcept { return nullptr; }
};

struct not_a_buffer
{
    std::size_t size() const noexcept;
    char* data() const noexcept;
};

struct asio_mutable_buffers
{
    asio_mutable_buffer const* begin() const noexcept;
    asio_mutable_buffer const* end() const noexcept;
};

struct asio_const_buffers
{
    asio_const_buffer const* begin() const noexcept;
    asio_const_buffer const* end() const noexcept;
};

BOOST_STATIC_ASSERT(  is_const_buffer    <const_buffer>::value);
BOOST_STATIC_ASSERT(  is_const_buffer    <mutable_buffer>::value);
BOOST_STATIC_ASSERT(  is_const_buffer    <asio_const_buffer>::value);
BOOST_STATIC_ASSERT(! is_const_buffer    <not_a_buffer>::value);

BOOST_STATIC_ASSERT(! is_mutable_buffer  <const_buffer>::value);
BOOST_STATIC_ASSERT(  is_mutable_buffer  <mutable_buffer>::value);
BOOST_STATIC_ASSERT(  is_mutable_buffer  <asio_mutable_buffer>::value);
BOOST_STATIC_ASSERT(! is_mutable_buffer  <not_a_buffer>::value);

BOOST_STATIC_ASSERT(  is_const_buffers   <const_buffer>::value);
BOOST_STATIC_ASSERT(  is_const_buffers   <mutable_buffer>::value);
BOOST_STATIC_ASSERT(  is_const_buffers   <asio_const_buffers>::value);
BOOST_STATIC_ASSERT(  is_const_buffers   <asio_mutable_buffers>::value);

BOOST_STATIC_ASSERT(  is_mutable_buffers <mutable_buffer>::value);
BOOST_STATIC_ASSERT(  is_mutable_buffers <asio_mutable_buffers>::value);
BOOST_STATIC_ASSERT(! is_mutable_buffers <asio_const_buffers>::value);

BOOST_STATIC_ASSERT(  is_const_buffers   <const_buffers_pair>::value);
BOOST_STATIC_ASSERT(  is_const_buffers   <mutable_buffers_pair>::value);
BOOST_STATIC_ASSERT(! is_mutable_buffers <const_buffers_pair>::value);
BOOST_STATIC_ASSERT(  is_mutable_buffers <mutable_buffers_pair>::value);

BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, const_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, mutable_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, asio_const_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, asio_mutable_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<mutable_buffer, mutable_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<mutable_buffer, asio_mutable_buffer>::value);

struct buffer_test
{
    struct test_mutable
    {
        test_mutable() = default;

        test_mutable(
            void* p,
            std::size_t n) noexcept
            : p_(p)
            , n_(n)
        {
        }

        void*
        data() const noexcept
        {
            return p_;
        }

        std::size_t
        size() const noexcept
        {
            return n_;
        }

    private:
        void* p_ = nullptr;
        std::size_t n_ = 0;
    };

    struct test_const
    {
        test_const() = default;

        test_const(
            void const* p,
            std::size_t n) noexcept
            : p_(p)
            , n_(n)
        {
        }

        void const*
        data() const noexcept
        {
            return p_;
        }

        std::size_t
        size() const noexcept
        {
            return n_;
        }

    private:
        void const* p_ = nullptr;
        std::size_t n_ = 0;
    };

    void
    testMutableBuffer()
    {
        // mutable_buffer()
        {
            mutable_buffer b;
            BOOST_TEST(b.size() == 0);
        }

        // mutable_buffer(void*, std::size_t)
        {
            char tmp[10];
            mutable_buffer b(tmp, sizeof(tmp));
            BOOST_TEST(b.size() == sizeof(tmp));
            BOOST_TEST_EQ(b.data(), tmp);
        }

        // mutable_buffer(mutable_buffer const&)
        {
            char tmp[10];
            mutable_buffer b(tmp, sizeof(tmp));
            BOOST_TEST(b.size() == sizeof(tmp));
            BOOST_TEST_EQ(b.data(), tmp);
        }

        // mutable_buffer(MutableBuffer)
        {
            char tmp[10];
            mutable_buffer b(
                test_mutable(tmp, sizeof(tmp)));
            BOOST_TEST(b.size() == sizeof(tmp));
            BOOST_TEST_EQ(b.data(), tmp);
        }

        // operator=(mutable_buffer const&)
        {
            {
                mutable_buffer b;
                char tmp[10];
                b = mutable_buffer(tmp, sizeof(tmp));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
            {
                mutable_buffer b;
                char tmp[10];
                b = test_mutable(tmp, sizeof(tmp));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
        }

        // data()
        // size()
        // operator+=
        {
            char tmp[10];
            mutable_buffer b(tmp, sizeof(tmp));
            b += 3;
            BOOST_TEST(
                b.size() == sizeof(tmp) - 3);
            BOOST_TEST_EQ(b.data(), tmp + 3);
        }

        // operator+
        {
            {
                char tmp[10];
                mutable_buffer b0(tmp, sizeof(tmp));
                auto b = b0 + 3;
                BOOST_TEST(
                    b.size() == sizeof(tmp) - 3);
                BOOST_TEST_EQ(b.data(), tmp + 3);
            }
            {
                char tmp[10];
                mutable_buffer b0(tmp, sizeof(tmp));
                auto b = 3 + b0;
                BOOST_TEST(
                    b.size() == sizeof(tmp) - 3);
                BOOST_TEST_EQ(b.data(), tmp + 3);
            }
        }
    }

    void
    testConstBuffer()
    {
        // const_buffer()
        {
            const_buffer b;
            BOOST_TEST(b.size() == 0);
        }

        // const_buffer(void const*, std::size_t)
        {
            char tmp[10];
            const_buffer b(tmp, sizeof(tmp));
            BOOST_TEST(b.size() == sizeof(tmp));
            BOOST_TEST_EQ(b.data(), tmp);
        }

        // const_buffer(const_buffer const&)
        {
            char tmp[10];
            const_buffer b(tmp, sizeof(tmp));
            BOOST_TEST(b.size() == sizeof(tmp));
            BOOST_TEST_EQ(b.data(), tmp);
        }

        // const_buffer(ConstBuffer)
        {
            {
                char tmp[10];
                const_buffer b(mutable_buffer(
                    tmp, sizeof(tmp)));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
            {
                char tmp[10];
                const_buffer b(
                    test_const(tmp, sizeof(tmp)));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
            {
                char tmp[10];
                const_buffer b(
                    test_mutable(tmp, sizeof(tmp)));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
        }

        // operator=(const_buffer const&)
        {
            {
                const_buffer b;
                char tmp[10];
                b = const_buffer(tmp, sizeof(tmp));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
            {
                const_buffer b;
                char tmp[10];
                b = mutable_buffer(tmp, sizeof(tmp));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
            {
                const_buffer b;
                char tmp[10];
                b = test_const(tmp, sizeof(tmp));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
            {
                const_buffer b;
                char tmp[10];
                b = test_mutable(tmp, sizeof(tmp));
                BOOST_TEST(b.size() == sizeof(tmp));
                BOOST_TEST_EQ(b.data(), tmp);
            }
        }

        // data()
        // size()
        // operator+=
        {
            char tmp[10];
            const_buffer b(tmp, sizeof(tmp));
            b += 3;
            BOOST_TEST(
                b.size() == sizeof(tmp) - 3);
            BOOST_TEST_EQ(b.data(), tmp + 3);
        }

        // operator+
        {
            {
                char tmp[10];
                const_buffer b0(tmp, sizeof(tmp));
                auto b = b0 + 3;
                BOOST_TEST(
                    b.size() == sizeof(tmp) - 3);
                BOOST_TEST_EQ(b.data(), tmp + 3);
            }
            {
                char tmp[10];
                const_buffer b0(tmp, sizeof(tmp));
                auto b = 3 + b0;
                BOOST_TEST(
                    b.size() == sizeof(tmp) - 3);
                BOOST_TEST_EQ(b.data(), tmp + 3);
            }
        }
    }

    void
    testBufferSize()
    {
        {
            char tmp[10];
            const_buffer b(tmp, sizeof(tmp));
            BOOST_TEST_EQ(buffer_size(b), 10);
            b += 3;
            BOOST_TEST_EQ(buffer_size(b), 7);
        }
        {
            char tmp[10];
            mutable_buffer b(tmp, sizeof(tmp));
            BOOST_TEST_EQ(buffer_size(b), 10);
            b += 3;
            BOOST_TEST_EQ(buffer_size(b), 7);
        }
        {
            char a[7];
            char b[11];
            const_buffers_pair p(
                const_buffer(a, sizeof(a)),
                const_buffer(b, sizeof(b)));
            BOOST_TEST_EQ(
                buffer_size(p),
                sizeof(a) + sizeof(b));
        }
    }

    void
    testBufferCopy()
    {
        string_view s =
            "Howdy partner";
        auto const N = s.size();
        for(std::size_t i = 0;
            i < N; ++i)
        {
            for(std::size_t j = 0;
                j < N; ++j)
            {
                for(std::size_t k = 0;
                    k < N + 2; ++k)
                {
                    const_buffers_pair p0(
                        const_buffer(
                            s.data(), i),
                        const_buffer(
                            s.data() + i, N - i));
                    char tmp[13];
                    std::memset(tmp, 0, sizeof(tmp));
                    mutable_buffers_pair p1(
                        mutable_buffer(
                            tmp, j),
                        mutable_buffer(
                            tmp + j, N - j));
                    auto const n = buffer_copy(
                        p1, p0, k);
                    BOOST_TEST_LE(n, N);
                    BOOST_TEST_EQ(
                        s.substr(0, n),
                        string_view(tmp, n));
                }
            }
        }
    }

    void
    run()
    {
        testMutableBuffer();
        testConstBuffer();
        testBufferSize();
        testBufferCopy();
    }
};

TEST_SUITE(
    buffer_test,
    "boost.http_proto.buffer");

} // http_proto
} // boost
