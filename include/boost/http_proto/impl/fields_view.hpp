//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_HPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_HPP

#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

class fields_view::iterator
{
    fields_view const* f_ = nullptr;
    char const* it_ = nullptr;

    off_t i_ = 0;
    off_t np_;
    off_t nn_;
    off_t vp_;
    off_t vn_;
    field id_;

    friend class fields;
    friend class fields_view;

    void
    read() noexcept;

    iterator(
        fields_view const* f,
        std::size_t i) noexcept;

public:
    using value_type =
        fields_view::value_type;
    using reference =
        fields_view::reference;
    using pointer = void const*;
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

class fields_view::subrange
{
    fields_view::iterator it_;
    fields_view::iterator end_;
    field id_;

    friend class fields_view;

    BOOST_HTTP_PROTO_DECL
    subrange(
        fields_view::iterator it,
        fields_view::iterator end) noexcept;

public:
    class iterator;

    using value_type =
        fields_view::value_type;
    using reference =
        fields_view::reference;
    using pointer = void const*;
    using iterator_category =
        std::forward_iterator_tag;

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

class fields_view::subrange::iterator
{
    fields_view::iterator it_;
    fields_view::iterator end_;
    field id_;

    friend class fields_view::subrange;

    iterator(
        fields_view::iterator it,
        fields_view::iterator end,
        field id) noexcept;

public:
    using value_type =
        fields_view::value_type;
    using reference =
        fields_view::reference;
    using pointer = void const*;
    using iterator_category =
        std::forward_iterator_tag;

    iterator(
        iterator const&) = default;

    iterator&
    operator=(
        iterator const&) = default;

    iterator() = default;

    operator
    fields_view::iterator() const noexcept
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
    fields_view::subrange r,
    Allocator const& a)
{
    auto it = r.begin();
    auto const end = r.end();
    if(it == end)
        return urls::const_string();
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
