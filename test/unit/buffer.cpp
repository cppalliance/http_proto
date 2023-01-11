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

template<class T>
using make_buffers_type =
    decltype(make_buffers(std::declval<T>()));

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

BOOST_STATIC_ASSERT(  is_const_buffers   <make_buffers_type<const_buffer>>::value);
BOOST_STATIC_ASSERT(  is_const_buffers   <make_buffers_type<mutable_buffer>>::value);
BOOST_STATIC_ASSERT(! is_mutable_buffers <make_buffers_type<const_buffer>>::value);
BOOST_STATIC_ASSERT(  is_mutable_buffers <make_buffers_type<mutable_buffer>>::value);

BOOST_STATIC_ASSERT(std::is_same<const_buffers_1&&, make_buffers_type<const_buffers_1>>::value);
BOOST_STATIC_ASSERT(std::is_same<mutable_buffers_1&&, make_buffers_type<mutable_buffers_1>>::value);
BOOST_STATIC_ASSERT(std::is_same<const_buffers_1 const&&, make_buffers_type<const_buffers_1 const>>::value);
BOOST_STATIC_ASSERT(std::is_same<mutable_buffers_1 const&&, make_buffers_type<mutable_buffers_1 const>>::value);

BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, const_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, mutable_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, asio_const_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<const_buffer, asio_mutable_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<mutable_buffer, mutable_buffer>::value);
BOOST_STATIC_ASSERT(std::is_constructible<mutable_buffer, asio_mutable_buffer>::value);

struct buffer_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    buffer_test,
    "boost.http_proto.buffer");

} // http_proto
} // boost
