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
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace boost {
namespace http_proto {

template<class BNF>
class bnf_range
{
    string_view s_;

public:
    using bnf_type = BNF;

    explicit
    bnf_range(
        string_view s)
        : s_(s)
    {
    }

    class iterator
    {
        char const* next_;
        char const* end_;
        typename BNF::state st_;

        friend class bnf_range;

        explicit
        iterator(string_view s)
            : end_(&*s.end())
        {
            error_code ec;
            next_ = BNF::begin(
                st_, s.data(), end_, ec);
        }

        explicit
        iterator(char const* end)
            : next_(nullptr)
            , end_(end)
        {
        }

    public:
        using value_type =
            decltype(std::declval<
                BNF::state>().value);
        using pointer = value_type const*;
        using reference = value_type const&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

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

        reference
        operator*() const
        {
            return st_.value;
        }

        pointer
        operator->() const
        {
            return &st_.value;
        }

        iterator&
        operator++()
        {
            error_code ec;
            next_ = BNF::increment(
                st_, next_, end_, ec);
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

    iterator
    begin() const
    {
        return iterator(s_);
    }

    iterator
    end() const
    {
        return iterator(
            &*s_.end());
    }
};

} // http_proto
} // boost

#endif
