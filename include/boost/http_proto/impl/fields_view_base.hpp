//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_BASE_HPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_BASE_HPP

#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

class fields_view_base::iterator
{
    fields_view_base const* f_ = nullptr;
    char const* it_ = nullptr;

    off_t i_ = 0;
    off_t np_;
    off_t nn_;
    off_t vp_;
    off_t vn_;
    field id_;

    friend class fields_base;
    friend class fields_view_base;

    void
    read() noexcept;

    iterator(
        fields_view_base const* f,
        std::size_t i) noexcept;

public:
    using value_type =
        fields_view_base::value_type;
    using reference =
        fields_view_base::reference;
    using pointer = void const*;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    iterator(
        iterator const&) = default;

    iterator&
    operator=(
        iterator const&) = default;

    iterator() = default;

    bool
    operator==(
        iterator const& other) const noexcept
    {
        // can't compare iterators
        // from different containers
        BOOST_ASSERT(f_ == other.f_);
        return i_ == other.i_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    BOOST_HTTP_PROTO_DECL
    reference
    operator*() const noexcept;

    reference
    operator->() const noexcept
    {
        return *(*this);
    }

    BOOST_HTTP_PROTO_DECL
    iterator&
    operator++() noexcept;

    iterator
    operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }
};

//------------------------------------------------

class fields_view_base::subrange
{
    fields_view_base::iterator it_;
    fields_view_base::iterator end_;
    field id_;

    friend class fields_view;
    friend class fields_view_base;

    BOOST_HTTP_PROTO_DECL
    subrange(
        fields_view_base::iterator it,
        fields_view_base::iterator end) noexcept;

public:
    class iterator;

    using value_type =
        fields_view_base::value_type;
    using reference =
        fields_view_base::reference;
    using const_reference =
        fields_view_base::reference;
    using size_type = std::size_t;
    using difference_type =
        std::ptrdiff_t;

    BOOST_HTTP_PROTO_DECL
    subrange(
        subrange const&) noexcept;

    BOOST_HTTP_PROTO_DECL
    subrange& operator=(
        subrange const&) noexcept;

    /** Constructor

        Default-constructed subranges are empty.
    */
    BOOST_HTTP_PROTO_DECL
    subrange() noexcept;

    BOOST_HTTP_PROTO_DECL
    iterator
    begin() const noexcept;

    BOOST_HTTP_PROTO_DECL
    iterator
    end() const noexcept;
};

//------------------------------------------------

class fields_view_base::subrange::iterator
{
    fields_view_base::iterator it_;
    fields_view_base::iterator end_;
    field id_;

    friend class fields_view_base::subrange;

    iterator(
        fields_view_base::iterator it,
        fields_view_base::iterator end,
        field id) noexcept;

public:
    using value_type =
        fields_view_base::value_type;
    using reference =
        fields_view_base::reference;
    using pointer = void const*;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    iterator(
        iterator const&) = default;

    iterator&
    operator=(
        iterator const&) = default;

    iterator() = default;

    operator
    fields_view_base::iterator() const noexcept
    {
        return it_;
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        // can't compare iterators
        // from different containers
        BOOST_ASSERT(
            end_ == other.end_);
        return it_ == other.it_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    reference
    operator*() const noexcept
    {
        return *it_;
    }

    reference
    operator->() const noexcept
    {
        return it_.operator->();
    }

    BOOST_HTTP_PROTO_DECL
    iterator&
    operator++() noexcept;

    iterator
    operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }
};

//------------------------------------------------

template<class Allocator>
urls::const_string
make_list(
    fields_view_base::subrange const& r,
    Allocator const& a)
{
    auto it = r.begin();
    auto const end = r.end();
    if(it == end)
        return {};
    // measure
    std::size_t n = 0;
    n += it->value.size();
    while(++it != end)
        n += 1 + it->value.size();
    // output
    it = r.begin();
    return urls::const_string(n, a,
        [&it, &end]
        (std::size_t, char* dest)
        {
            auto const n =
                it->value.size();
            std::memcpy(
                dest,
                it->value.data(),
                n);
            while(++it != end)
            {
                dest += n;
                *dest++ = ',';
                std::memcpy(
                    dest,
                    it->value.data(),
                    n);
            }
        });
}

} // http_proto
} // boost

#endif
