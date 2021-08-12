//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_TRIVIAL_OPTIONAL_HPP
#define BOOST_HTTP_PROTO_TRIVIAL_OPTIONAL_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/assert.hpp>
#include <boost/none_t.hpp>
#include <boost/static_assert.hpp>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace boost {
namespace http_proto {

template<class T>
class trivial_optional
{
    BOOST_STATIC_ASSERT(
        std::is_trivially_destructible<
            T>::value);

    BOOST_STATIC_ASSERT(
        std::is_nothrow_copy_constructible<
            T>::value);

    BOOST_STATIC_ASSERT(
        std::is_destructible<
            T>::value);

    union U
    {
        T v;
        U(){}
        ~U(){}
    };
    U u_;
    bool b_ = false;

public:
    using value_type = T;

    //
    // Special Members
    //

    ~trivial_optional()
    {
        if(b_)
            u_.v.~T();
    }

    constexpr
    trivial_optional() = default;

    constexpr
    trivial_optional(none_t) noexcept
    {
    }

    constexpr
    trivial_optional(
        trivial_optional const& other) noexcept
    {
        if(other.b_)
        {
            ::new(&u_.v) T(other.u_.v);
            b_ = true;
        }
        else
        {
            b_ = false;
        }
    }

    trivial_optional(
        T const& value) noexcept
    {
        ::new(&u_.v) T(value);
        b_ = true;
    }

    trivial_optional&
    operator=(none_t) noexcept
    {
        if(b_)
        {
            u_.v.~T();
            b_ = false;
        }
        return *this;
    }

    trivial_optional&
    operator=(trivial_optional<
        T> const& other) noexcept
    {
        this->~trivial_optional();
        ::new(this) trivial_optional(other);
        return *this;
    }

    trivial_optional&
    operator=(T const& v) noexcept
    {
        this->~trivial_optional();
        ::new(this) trivial_optional(v);
        return *this;
    }

    //
    // Observers
    //

    constexpr
    T const*
    operator->() const noexcept
    {
        BOOST_ASSERT(b_);
        return &u_.v;
    }

    constexpr
    T*
    operator->() noexcept
    {
        BOOST_ASSERT(b_);
        return &u_.v;
    }
 
    constexpr
    T const&
    operator*() const noexcept
    {
        BOOST_ASSERT(b_);
        return u_.v;
    }
     
    constexpr
    T&
    operator*() noexcept
    {
        BOOST_ASSERT(b_);
        return u_.v;
    }
 
    constexpr
    explicit
    operator bool() const noexcept
    {
        return b_;
    }
    
    constexpr
    bool
    has_value() const noexcept
    {
        return b_;
    }

    constexpr
    T const&
    value() const
    {
        if (! b_)
            detail::throw_invalid_argument(
                "bad optional access",
                BOOST_CURRENT_LOCATION);
        return u_.v;
    }
 
    constexpr
    T&
    value()
    {
        if (! b_)
            detail::throw_invalid_argument(
                "bad optional access",
                BOOST_CURRENT_LOCATION);
        return u_.v;
    }

    template<class U>
    constexpr T
    value_or(U&& default_value) const
    {
        if(b_)
            return u_.v;
        return default_value;
    }

    //
    // Modifiers
    //

    constexpr
    void
    reset() noexcept
    {
        if(b_)
        {
            u_.v.~T();
            b_ = false;
        }
    }

    template<class... Args>
    constexpr
    T&
    emplace(Args&&... args) noexcept
    {
        if(b_)
            u_.v.~T();
        ::new(&u_.v) T(std::forward<
            Args>(args)...);
        b_ = true;
        return u_.v;
    }

    template<class U, class... Args>
    constexpr
    T&
    emplace(
        std::initializer_list<U> ilist,
        Args&&... args) noexcept
    {
        if(b_)
            u_.v.~T();
        ::new(&u_.v) T(ilist,
            std::forward<Args>(args)...);
        b_ = true;
        return u_.v;
    }

    //
    // Non-member functions
    //

    friend
    constexpr bool operator==(
        trivial_optional<T> const& lhs,
        trivial_optional<T> const& rhs) noexcept
    {
        if(! lhs.b_)
            return ! rhs.b_;
        if(! rhs.b_)
            return false;
        return lhs.u_.v == rhs.u_.v;
    }

    friend
    constexpr bool operator!=(
        trivial_optional<T> const& lhs,
        trivial_optional<T> const& rhs) noexcept
    {
        return ! (lhs == rhs);
    }
};

} // http_proto
} // boost

#endif
