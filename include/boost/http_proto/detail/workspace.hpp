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

        virtual ~any() = 0;
        virtual void* move(
            void* dest) noexcept = 0;
    };

    template<class T>
    struct alignas(alignof(::max_align_t))
        any_T : any
    {
        T t;

        any_T() = delete;
        any_T(any_T&&) = default;

        template<class... Args>
        explicit
        any_T(
            int,
            Args&&... args)
            : t(std::forward<
                Args>(args)...)
        {
        }

        void*
        move(void* head) noexcept
        {
            return ::new(reinterpret_cast<
                void*>(reinterpret_cast<
                    std::uintptr_t>(head) -
                        sizeof(*this))) any_T(
                std::move(*this));
        }
    };

    char* buf_ = nullptr;
    any* head_ = nullptr;
    std::size_t cap_ = 0;

public:
    ~workspace()
    {
        clear();
    }

    workspace() = default;

    workspace(
        std::size_t n)
    {
        reserve(n);
    }

    void
    clear()
    {
        while(head_)
        {
            auto next = head_->next;
            head_->~any();
            head_ = next;
        }
    }

    void
    reserve(
        std::size_t n)
    {
        if(cap_ >= n)
            return;
        // VFALCO growth policy
        // align down for ::max_align_t
        auto cap = n;
        auto buf = new char[cap];
        if(buf_)
        {
            // move everything to new buf
            auto head = head_;
            void* top = reinterpret_cast<
                void*>(reinterpret_cast<
                    std::uintptr_t>(
                        buf) + cap);
            while(head)
            {
                auto next = head->next;
                top = head->move(top);
                head = next;
            }
            head_ = reinterpret_cast<
                any*>(top);
            delete[] buf_;
        }
        buf_ = buf;
        cap_ = cap;
    }

    template<class T, class... Args>
    friend
    T& push(workspace&, Args&&...);

    void
    pop()
    {
        BOOST_ASSERT(head_);
        auto next = head_->next;
        head_->~any();
        head_ = next;
    }

private:
    void*
    addr_for(std::size_t n)
    {
        if(head_)
            return reinterpret_cast<void*>(
                reinterpret_cast<
                std::uintptr_t>(head_) - n);
        return reinterpret_cast<void*>(
            reinterpret_cast<
            std::uintptr_t>(buf_) + cap_ - n);
    }
};

template<class T, class... Args>
T&
push(workspace& ws, Args&&... args)
{
    using U = workspace::any_T<T>;
    auto p = ::new(ws.addr_for(
        sizeof(U))) U(0, std::forward<
            Args>(args)...);
    p->next = ws.head_;
    ws.head_ = p;
    return p->t;
}

template<class T, class... Args>
T*
push_array(
    workspace& ws, std::size_t n)
{
    return nullptr;
}

} // detail
} // http_proto
} // boost

#endif
