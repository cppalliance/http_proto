//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_HEADERS_VIEW_HPP
#define BOOST_HTTP_PROTO_IMPL_HEADERS_VIEW_HPP

#include <boost/http_proto/arrow_proxy.hpp>
#include <boost/assert.hpp>
#include <iterator>

namespace boost {
namespace http_proto {

class headers_view::iterator
{
    headers_view const* h_ = nullptr;
    std::size_t i_ = 0;

    friend class headers_view;
    friend class headers_view::subrange;

    iterator(
        headers_view const* h,
        std::size_t i) noexcept
        : h_(h)
        , i_(i)
    {
    }

public:
    using value_type = typename
        headers_view::value_type;
    using pointer = value_type;
    using reference = value_type;
    using iterator_category =
        std::bidirectional_iterator_tag;

    iterator() = default;

    bool
    operator==(
        iterator const& other) const
    {
        return
            h_ == other.h_ &&
            i_ == other.i_;
    }

    bool
    operator!=(
        iterator const& other) const
    {
        return !(*this == other);
    }

    value_type
    operator*() const noexcept
    {
        return (*h_)[i_];
    }

    arrow_proxy<value_type>
    operator->() const noexcept
    {
        return arrow_proxy<
            value_type>{ (*h_)[i_] };
    }

    iterator&
    operator++()
    {
        BOOST_ASSERT(
            i_ < h_->count_);
        ++i_;
        return *this;
    }

    iterator
    operator++(int)
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    iterator&
    operator--()
    {
        BOOST_ASSERT(
            i_ != 0);
        --i_;
        return *this;
    }

    iterator
    operator--(int)
    {
        auto temp = *this;
        --(*this);
        return temp;
    }
};

//------------------------------------------------

auto
headers_view::
begin() const noexcept ->
    iterator
{
    return iterator(this, 0);
}

auto
headers_view::
end() const noexcept ->
    iterator
{
    return iterator(this, count_);
}

//------------------------------------------------

class headers_view::subrange::iterator
{
    headers_view const* h_ = nullptr;
    std::size_t i_ = 0;

    friend class subrange;

    iterator(
        headers_view const* h,
        std::size_t i) noexcept
        : h_(h)
        , i_(i)
    {
    }

public:
    using value_type =
        typename headers_view::value_type;
    using pointer = value_type;
    using reference = value_type;
    using iterator_category =
        std::forward_iterator_tag;

    iterator() noexcept = default;

    bool
    operator==(
        iterator const& other) const
    {
        return
            h_ == other.h_ &&
            i_ == other.i_;
    }

    bool
    operator!=(
        iterator const& other) const
    {
        return !(*this == other);
    }

    value_type
    operator*() const noexcept
    {
        return (*h_)[i_];
    }

    arrow_proxy<value_type>
    operator->() const noexcept
    {
        return arrow_proxy<
            value_type>{ **this };
    }

    BOOST_HTTP_PROTO_DECL
    iterator&
    operator++() noexcept;

    iterator
    operator++(int)
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }
};

//------------------------------------------------

headers_view::
subrange::
subrange(
    headers_view const* h,
    std::size_t first) noexcept
    : h_(h)
    , first_(first)
{
}

auto
headers_view::
subrange::
begin() const noexcept ->
    iterator
{
    return iterator(
        h_, first_);
}

auto
headers_view::
subrange::
end() const noexcept ->
    iterator
{
    return iterator(
        h_, h_->size());
}

//------------------------------------------------

} // http_proto
} // boost

#endif
