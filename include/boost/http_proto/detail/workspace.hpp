//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_WORKSPACE_HPP
#define BOOST_HTTP_PROTO_DETAIL_WORKSPACE_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/assert.hpp>
#include <cstdlib>
#include <new>
#include <utility>
#include <stddef.h> // ::max_align_t

namespace boost {
namespace http_proto {
namespace detail {

class workspace
{
    struct any
    {
        any* next = nullptr;

        BOOST_HTTP_PROTO_DECL
        virtual ~any() = 0;
    };

    template<class T>
    struct alignas(alignof(::max_align_t))
        any_t : any
    {
        T t_;

        any_t() = delete;
        any_t(any_t&&) = default;

        explicit
        any_t(
            T&& t)
            : t_(std::move(t))
        {
        }

        explicit
        any_t(
            T const& t)
            : t_(t)
        {
        }
    };

    template<class T>
    struct alignas(alignof(::max_align_t))
        any_n : any
    {
        std::size_t n_ = 0;

        any_n() = default;

        ~any_n()
        {
            for(std::size_t i = n_;
                    i-- > 0;)
                data()[i].~T();
        }

        any_n(
            std::size_t n,
            T const& t)
            : any_n()
        {
            while(n_ < n)
            {
                new(&data()[n_]) T(t);
                ++n_;
            }
        }

        T*
        data() noexcept
        {
            return
                reinterpret_cast<T*>(
                    this + 1);
        }
    };

    unsigned char* begin_ = nullptr;
    unsigned char* end_ = nullptr;
    unsigned char* head_ = nullptr;

public:
    workspace() = default;

    ~workspace()
    {
        clear();
        delete[] begin_;
    }

    explicit
    workspace(
        std::size_t n)
        : begin_(new unsigned char[n])
        , end_(begin_ + n)
        , head_(end_)
    {
    }

    void*
    data() noexcept
    {
        return begin_;
    }

    std::size_t
    size() const noexcept
    {
        return head_ - begin_;
    }

    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    BOOST_HTTP_PROTO_DECL
    void*
    reserve(std::size_t n);

    template<class T>
    auto
    push(T&& t) ->
        typename std::decay<T>::type&
    {
        using U = any_t<typename
            std::decay<T>::type>;
        auto p = ::new(bump_down(
            sizeof(U), alignof(U))) U(
                std::forward<T>(t));
        p->next = reinterpret_cast<
            any*>(head_);
        head_ = reinterpret_cast<
            unsigned char*>(p);
        return p->t_;
    }

    template<class T>
    T*
    push_array(
        std::size_t n,
        T const& t)
    {
        using U = any_n<T>;
        auto p = ::new(bump_down(
            sizeof(U) + n * sizeof(T),
                alignof(::max_align_t))) U(n, t);
        p->next = reinterpret_cast<
            any*>(head_);
        head_ = reinterpret_cast<
            unsigned char*>(p);
        return p->data();
    }

private:
    // https://fitzgeraldnick.com/2019/11/01/always-bump-downwards.html
    void*
    bump_down(
        std::size_t size,
        std::size_t align)
    {
        BOOST_ASSERT(align > 0);
        BOOST_ASSERT(
            (align & (align - 1)) == 0);

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
};

} // detail
} // http_proto
} // boost

#endif
