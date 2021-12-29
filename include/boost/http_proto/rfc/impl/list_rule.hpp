//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_LIST_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_IMPL_LIST_RULE_HPP

#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/url/error.hpp>
#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/parse.hpp>
#include <cstdint>
#include <iterator>

namespace boost {
namespace http_proto {

namespace detail {

struct ows_comma
{
    friend
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        ows_comma) noexcept
    {
        auto const start = it;
        it = grammar::find_if_not(
            it, end, ws);
        if(it == end)
        {
            // expected comma
            it = start;
            ec = grammar::error::syntax;
            return false;
        }
        if(*it != ',')
        {
            // expected comma
            it = start;
            ec = grammar::error::syntax;
            return false;
        }
        ++it;
        return true;
    }
};

struct comma_ows
{
    friend
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        comma_ows) noexcept
    {
        if(it == end)
        {
            // expected comma
            ec = grammar::error::syntax;
            return false;
        }
        if(*it != ',')
        {
            // expected comma
            ec = grammar::error::syntax;
            return false;
        }
        it = grammar::find_if_not(
            it + 1, end, ws);
        return true;
    }
};

} // detail

//------------------------------------------------

template<class T>
class list_rule_base<T>::iterator
{
    friend class list_rule_base<T>;

    T t_;
    char const* next_ = nullptr;
    char const* end_ = nullptr;

    iterator(
        char const* it,
        char const* end) noexcept
        : next_(it)
        , end_(end)
    {
        if(next_ != nullptr)
        {
            error_code ec;
            list_rule_base<T>::begin(
                next_, end_, ec, t_);
            if(ec == grammar::error::end)
                next_ = nullptr;
        }
    }

public:
#ifdef BOOST_HTTP_PROTO_DOCS
    using value_type = __see_below__;
#else
    using value_type = decltype(*t_);
#endif
    using reference = value_type const&;
    using pointer = void const*;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    iterator() = default;

    value_type
    operator*() const noexcept
    {
        return *t_;
    }

    iterator&
    operator++()
    {
        error_code ec;
        list_rule_base<T>::increment(
            next_, end_, ec, t_);
        if(ec == grammar::error::end)
            next_ = nullptr;
        return *this;
    }

    iterator
    operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return
            next_ == other.next_ &&
            end_ == other.end_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }
};

//------------------------------------------------

template<class T>
bool
list_rule_base<T>::
begin(
    char const*& it,
    char const* end,
    error_code& ec,
    T& t)
{
    using grammar::parse;
    if(it == end)
    {
        // empty list
        ec = grammar::error::end;
        return false;
    }

    // ( element ) ;most common
    if(parse(it, end, ec, t))
        return true;
    ec = {};

    // *( "," OWS ) element
    for(;;)
    {
        if(! parse(it, end, ec,
            detail::comma_ows{}))
        {
            ec = {};
            break;
        }
    }
    if(! parse(it, end, ec, t))
    {
        // expected element
        ec = grammar::error::syntax;
        return false;
    }

    return true;
}

template<class T>
bool
list_rule_base<T>::
increment(
    char const*& it,
    char const* end,
    error_code& ec,
    T& t)
{
    using grammar::parse;

    // *( OWS "," )
    if(! parse(it, end, ec,
        detail::ows_comma{}))
    {
        ec = grammar::error::end;
        return false;
    }
    for(;;)
    {
        if(! parse(it, end, ec,
            detail::ows_comma{}))
        {
            ec = {};
            break;
        }
    }
    if(it == end)
    {
        ec = grammar::error::end;
        return false;
    }

    // [ OWS element ]
    auto const start = it;
    it = grammar::find_if_not(it, end, ws);
    if(! parse(it, end, ec, t))
    {
        it = start;
        ec = grammar::error::end;
        return false;
    }
    return true;
}

template<class T>
bool
list_rule_base<T>::
parse(
    char const*& it,
    char const* end,
    error_code& ec,
    std::size_t N,
    std::size_t M,
    std::size_t& n)
{
    T t;
    n = 0;
    if(list_rule_base<T>::begin(
        it, end, ec, t))
    {
        for(;;)
        {
            ++n;
            if(n > M)
            {
                // too many
                ec = grammar::error::syntax;
                return false;
            }
            if(! list_rule_base<T>::increment(
                it, end, ec, t))
                break;
        }
    }
    if(ec != grammar::error::end)
        return false;
    if(n < N)
    {
        // too few
        ec = grammar::error::syntax;
        return false;
    }
    ec = {};
    return true;
}

template<class T>
auto
list_rule_base<T>::
begin() const noexcept ->
    iterator
{
    return iterator(
        s_.begin(),
        s_.begin() + s_.size());
}

template<class T>
auto
list_rule_base<T>::
end() const noexcept ->
    iterator
{
    return iterator(
        nullptr,
        s_.begin() + s_.size());
}

//------------------------------------------------

template<
    class T,
    std::size_t N,
    std::size_t M>
list_rule<T, N, M>::
list_rule(string_view s)
{
    grammar::parse_string(
        s, *this);
}

template<
    class T,
    std::size_t N,
    std::size_t M>
bool
parse(
    char const*& it,
    char const* end,
    error_code& ec,
    list_rule<T, N, M>& t)
{
    std::size_t n;
    auto const start = it;
    if(! list_rule<T, N, M>::parse(
            it, end, ec, N, M, n))
        return false;
    t = list_rule<T, N, M>(string_view(
        start, it - start), n);
    return true;
}

} // http_proto
} // boost

#endif
