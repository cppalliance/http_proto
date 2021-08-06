//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_RANGE_HPP
#define BOOST_HTTP_PROTO_BNF_RANGE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace boost {
namespace http_proto {

template<class T>
class bnf_range
{
    string_view s_;

public:
    using type = T;

    class iterator;

    bnf_range(
        bnf_range const&) = default;
    bnf_range& operator=(
        bnf_range const&) = default;

    /** Default constructor

        Iteration of default constructed ranges
        is undefined.
    */
    bnf_range() = default;

    /** Constructor
    */
    explicit
    bnf_range(
        string_view s)
        : s_(s)
    {
    }

    inline iterator begin(
        error_code& ec) const;

    inline iterator begin() const;

    inline iterator end() const;

    inline void validate(
        error_code& ec) const;

    inline void validate() const;

    inline bool is_valid() const;
};

//------------------------------------------------

template<class T>
class bnf_range<T>::iterator
{
    char const* next_;
    char const* end_;
    T impl_;

    friend class bnf_range;

    explicit
    iterator(string_view s)
        : end_(&*s.end())
    {
        error_code ec;
        next_ = impl_.begin(
            s.data(), end_, ec);
        if(ec)
            detail::throw_system_error(ec,
                BOOST_CURRENT_LOCATION);
    }

    iterator(
        string_view s,
        error_code& ec)
        : end_(&*s.end())
    {
        next_ = impl_.begin(
            s.data(), end_, ec);
    }

    explicit
    iterator(char const* end)
        : next_(nullptr)
        , end_(end)
    {
    }

public:
    using value_type = decltype(
        std::declval<T>().value);
    using pointer = value_type;
    using reference = value_type;
    using iterator_category =
        std::forward_iterator_tag;

    iterator() noexcept
        : next_(nullptr)
        , end_(nullptr)
    {
    }

    bool
    operator==(
        iterator const& other) const
    {
        return
            next_ == other.next_ &&
            end_ == other.end_;
    }

    bool
    operator!=(
        iterator const& other) const
    {
        return !(*this == other);
    }

    value_type
    operator*() const
    {
        return impl_.value;
    }

    void
    increment(
        error_code& ec)
    {
        next_ = impl_.increment(
            next_, end_, ec);
    }

    iterator&
    operator++()
    {
        error_code ec;
        increment(ec);
        if(ec)
            detail::throw_system_error(ec,
                BOOST_CURRENT_LOCATION);
        return *this;
    }

    iterator
    operator++(int)
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }
};

template<class T>
auto
bnf_range<T>::
begin(error_code& ec) const ->
    iterator
{
    iterator it(s_, ec);
    if(! ec)
        return it;
    return iterator(
        &*s_.end());
}

template<class T>
auto
bnf_range<T>::
begin() const ->
    iterator
{
    return iterator(s_);
}

template<class T>
auto
bnf_range<T>::
end() const ->
    iterator
{
    return iterator(
        &*s_.end());
}

//------------------------------------------------

template<class T>
void
bnf_range<T>::
validate(
    error_code& ec) const
{
    auto const end_ = end();
    auto it = begin(ec);
    while(! ec)
    {
        if(it == end_)
            return;
        it.increment(ec);
    }
}

template<class T>
void
bnf_range<T>::
validate() const
{
    error_code ec;
    validate(ec);
    if(ec.failed())
        detail::throw_system_error(ec,
            BOOST_CURRENT_LOCATION);
}

template<class T>
bool
bnf_range<T>::
is_valid() const
{
    error_code ec;
    validate(ec);
    return ! ec.failed();
}

//------------------------------------------------

/** Return the valid prefix of s for the BNF T
*/
template<class T>
string_view
valid_prefix(
    string_view s)
{
    error_code ec;
    auto const end = &*s.end();
    auto pos = s.data();
    T impl;
    pos = impl.begin(pos, end, ec);
    // nothing valid
    if(ec)
        return { s.data(), 0 };
    // valid empty list
    if(! pos)
        return { s.data(), 0 };
    for(;;)
    {
        auto next = impl.increment(
            pos, end, ec);
        if(ec)
            break;
        if(! next)
        {
            pos = end;
            break;
        }
        pos = next;
    }
    return { s.data(), static_cast<
        std::size_t>(pos - s.data()) };
}

} // http_proto
} // boost

#endif
