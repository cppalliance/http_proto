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

//------------------------------------------------
//
// iterator
//
//------------------------------------------------

class fields_view_base::iterator
{
    detail::header const* ph_ = nullptr;
    std::size_t i_ = 0;

    friend class fields_base;
    friend class fields_view_base;

    iterator(
        detail::header const* ph,
        std::size_t i) noexcept
        : ph_(ph)
        , i_(i)
    {
    }

public:
    using value_type =
        fields_view_base::value_type;
    using reference =
        fields_view_base::reference;
    using pointer = reference;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    iterator() = default;
    iterator(iterator const&) = default;
    iterator& operator=(
        iterator const&) = default;

    bool
    operator==(
        iterator const& other) const noexcept
    {
        // If this assert goes off, it means you
        // are trying to compare iterators from
        // different containers, which is undefined!
        BOOST_ASSERT(ph_ == other.ph_);

        return i_ == other.i_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    BOOST_HTTP_PROTO_DECL
    reference const
    operator*() const noexcept;

    pointer const
    operator->() const noexcept
    {
        return *(*this);
    }

    iterator&
    operator++() noexcept
    {
        BOOST_ASSERT(i_ < ph_->count);
        ++i_;
        return *this;
    }

    iterator
    operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    iterator&
    operator--() noexcept
    {
        BOOST_ASSERT(i_ > 0);
        --i_;
        return *this;
    }

    iterator
    operator--(int) noexcept
    {
        auto temp = *this;
        --(*this);
        return temp;
    }
};

//------------------------------------------------
//
// subrange
//
//------------------------------------------------

class fields_view_base::subrange
{
    detail::header const* ph_ = nullptr;
    std::size_t i_ = 0;

    friend class fields_view;
    friend class fields_view_base;
    friend struct detail::header;

    subrange(
        detail::header const* ph,
        std::size_t i) noexcept
        : ph_(ph)
        , i_(i)
    {
    }

public:
    class iterator;
    using const_iterator = iterator;
    using value_type = std::string;
    using reference = string_view;
    using const_reference = reference;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    /** Constructor

        Default-constructed subranges are empty.
    */
    subrange() noexcept = default;

    subrange(subrange const&) noexcept = default;
    subrange& operator=(
        subrange const&) noexcept = default;

    iterator begin() const noexcept;
    iterator end() const noexcept;
};

//------------------------------------------------
//
// subrange::iterator
//
//------------------------------------------------

class fields_view_base::subrange::
    iterator
{
    detail::header const* ph_ = nullptr;
    std::size_t i_ = 0;
    std::size_t n_ = std::size_t(-1);

    friend class fields_view_base::subrange;

    BOOST_HTTP_PROTO_DECL
    iterator(
        detail::header const* ph,
        std::size_t i) noexcept;

public:
    using value_type = std::string;
    using reference = string_view;
    using pointer = void const*;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    iterator() = default;
    iterator(iterator const&) = default;
    iterator& operator=(
        iterator const&) = default;

    // conversion to regular iterator
    operator
    fields_view_base::
    iterator() const noexcept
    {
        return {ph_, i_};
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        // If this assert goes off, it means you
        // are trying to compare iterators from
        // different containers, which is undefined!
        BOOST_ASSERT(ph_ == other.ph_);

        return i_ == other.i_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    BOOST_HTTP_PROTO_DECL
    reference const
    operator*() const noexcept;

    reference const
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

inline
auto
fields_view_base::
subrange::
begin() const noexcept ->
    iterator
{
    return {ph_, i_};
}

inline
auto
fields_view_base::
subrange::
end() const noexcept ->
    iterator
{
    return {ph_, ph_->count};
}

//------------------------------------------------

inline
fields_view_base::
value_type::
operator
fields_view_base::
reference() const noexcept
{
    return reference{
        id, name, value};
}

//------------------------------------------------

inline
auto
fields_view_base::
begin() const noexcept ->
    iterator
{
    return iterator(ph_, 0);
}

inline
auto
fields_view_base::
end() const noexcept ->
    iterator
{
    return iterator(ph_, ph_->count);
}

inline
auto
fields_view_base::
rbegin() const noexcept ->
    reverse_iterator
{
    return reverse_iterator(end());
}

inline
auto
fields_view_base::
rend() const noexcept ->
    reverse_iterator
{
    return reverse_iterator(begin());
}

} // http_proto
} // boost

#endif
