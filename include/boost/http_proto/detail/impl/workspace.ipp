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
    , front_(begin_)
    , head_(begin_ + n)
    , back_(head_)
    , end_(head_)
{
}

workspace::
workspace(
    workspace&& other) noexcept
    : begin_(other.begin_)
    , front_(other.front_)
    , head_(other.end_)
    , back_(other.back_)
    , end_(other.end_)
{
    other.begin_ = nullptr;
    other.front_ = nullptr;
    other.head_ = nullptr;
    other.back_ = nullptr;
    other.end_ = nullptr;
}

void
workspace::
allocate(
    std::size_t n)
{
    // Cannot be empty
    if(n == 0)
        detail::throw_invalid_argument();

    // Already allocated
    if(begin_ != nullptr)
        detail::throw_logic_error();

    begin_ = new unsigned char[n];
    front_ = begin_;
    head_ = begin_ + n;
    back_ = head_;
    end_ = head_;
}

void
workspace::
clear() noexcept
{
    if(! begin_)
        return;

    auto const end =
        reinterpret_cast<
            any const*>(back_);
    auto p =
        reinterpret_cast<
            any const*>(head_);
    while(p != end)
    {
        auto next = p->next;
        p->~any();
        p = next;
    }
    front_ = begin_;
    head_ = end_;
    back_ = end_;
}

void*
workspace::
reserve_front(
    std::size_t n)
{
    // 
    // Requested size exceeds available space.
    // Note you can never reserve the last byte.
    if(n >= size())
        detail::throw_length_error();

    auto const p = front_;
    front_ += n ;
    return p;
}

void*
workspace::
reserve_back(
    std::size_t n)
{
    // can't reserve after acquire
    if(head_ != end_)
        detail::throw_logic_error();

    // can't reserve twice
    if(back_ != end_)
        detail::throw_logic_error();

    // over capacity
    std::size_t const lim =
        head_ - front_;
    if(n >= lim)
        detail::throw_length_error();

    head_ -= n;
    back_ = head_;
    return back_;
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
    BOOST_ASSERT(front_);

    auto ip0 = reinterpret_cast<
        std::uintptr_t>(front_);
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
