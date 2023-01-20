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

workspace::
any::
~any() = default;

workspace::
~workspace()
{
    if(begin_)
    {
        clear();
        delete[] begin_;
    }
}

workspace::
workspace(
    std::size_t n)
    : begin_(new unsigned char[n])
    , end_(begin_ + n)
    , head_(end_)
{
}

workspace::
workspace(
    workspace&& other) noexcept
    : begin_(other.begin_)
    , end_(other.end_)
    , head_(end_)
{
    other.begin_ = nullptr;
}

workspace&
workspace::
operator=(
    workspace&& other) noexcept
{
    // *this is not empty
    if(begin_ != nullptr)
        detail::throw_length_error();

    begin_ = other.begin_;
    end_ = other.end_;
    head_ = end_;
    other.begin_ = nullptr;
    return *this;
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
}

void*
workspace::
reserve(std::size_t n)
{
    BOOST_ASSERT(begin_);

    // Requested size exceeds available space.
    if(n > size())
        detail::throw_length_error();

    struct empty : any
    {
    };

    using U = empty;
    auto p = ::new(bump_down(
        sizeof(U) + n, alignof(
            ::max_align_t))) U;
    p->next = reinterpret_cast<
        any*>(head_);
    head_ = reinterpret_cast<
        unsigned char*>(p);
    return p + 1;
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
