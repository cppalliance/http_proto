//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_WORKSPACE_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_WORKSPACE_IPP

#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace detail {

/*  Layout

    The buffer is laid out thusly:

  base_         begin_        head_           end_

    |<- reserved ->|<- unused ->|<- acquired ->|
*/
workspace::
any::
~any() = default;

workspace::
~workspace()
{
    if(base_)
    {
        clear();
        delete[] base_;
    }
}

workspace::
workspace(
    std::size_t n)
    : base_(new unsigned char[n])
    , begin_(base_)
    , head_(base_ + n)
    , end_(head_)
{
}

workspace::
workspace(
    workspace&& other) noexcept
    : base_(other.base_)
    , begin_(other.begin_)
    , head_(other.end_)
    , end_(other.end_)
{
    other.base_ = nullptr;
    other.begin_ = nullptr;
    other.head_ = nullptr;
    other.end_ = nullptr;
}

void
workspace::
allocate(
    std::size_t n)
{
    // n == 0
    if(n == 0)
        detail::throw_invalid_argument();

    // this->size() > 0
    if(base_ != nullptr)
        detail::throw_logic_error();

    base_ = new unsigned char[n];
    begin_ = base_;
    head_ = base_ + n;
    end_ = head_;
}

void
workspace::
clear() noexcept
{
    BOOST_ASSERT(begin_);

    auto const end =
        reinterpret_cast<
            any const*>(end_);
    auto p =
        reinterpret_cast<
            any const*>(head_);
    while(p != end)
    {
        auto next = p->next;
        p->~any();
        p = next;
    }
    head_ = end_;
    begin_ = base_;
}

void
workspace::
reserve(std::size_t n)
{
    // Requested size exceeds available space.
    // Note you can never reserve the last byte.
    if(n >= size())
        detail::throw_length_error();

    base_ += n ;
}

// https://fitzgeraldnick.com/2019/11/01/always-bump-downwards.html
void*
workspace::
bump_down(
    std::size_t size,
    std::size_t align)
{
    BOOST_ASSERT(align > 0);
    BOOST_ASSERT(
        (align & (align - 1)) == 0);
    BOOST_ASSERT(begin_);

    auto ip0 = reinterpret_cast<
        std::uintptr_t>(begin_);
    auto ip = reinterpret_cast<
        std::uintptr_t>(head_);

    // If you get an exception here, it
    // means that a buffer was too small
    // for your workload. Increase the
    // buffer size.
    if(size > ip - ip0)
        detail::throw_bad_alloc();

    ip -= size;
    ip &= ~(align - 1);

    // If you get an exception here, it
    // means that a buffer was too small
    // for your workload. Increase the
    // buffer size.
    if(ip < ip0)
        detail::throw_bad_alloc();

    return reinterpret_cast<void*>(ip);
}

} // detail
} // http_proto
} // boost

#endif
