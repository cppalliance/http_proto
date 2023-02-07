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

/** A contiguous buffer of storage used by algorithms.

    Objects of this type retain ownership of a
    contiguous buffer of storage allocated upon
    construction. This storage is divided into
    three regions:

    @li The reserved area, which starts at the
        beginning of the buffer and can grow
        upwards towards the end of the buffer.

    @li The acquired area, which starts at the
        end of the buffer and can grow downwards
        towards the beginning of the buffer.

    @li The unused area, which starts from the
        end of the reserved area and stretches
        until the beginning of the acquired area.
*/
class workspace
{
    struct any
    {
        any* next = nullptr;

        BOOST_HTTP_PROTO_DECL
        virtual ~any() = 0;
    };

    unsigned char* base_ = nullptr;
    unsigned char* begin_ = nullptr;
    unsigned char* head_ = nullptr;
    unsigned char* end_ = nullptr;

public:
    /** Destructor.
    */
    ~workspace();

    /** Constructor.

        @param n The number of bytes of storage
            to allocate for the internal buffer.
    */
    explicit
    workspace(
        std::size_t n);

    /** Constructor.
    */
    workspace() = default;

    /** Constructor.
    */
    workspace(workspace&&) noexcept;

    /** Allocate internal storage.

        @throws std::logic_error this->size() > 0

        @throws std::invalid_argument n == 0
    */
    void
    allocate(
        std::size_t n);

    /** Return a pointer to the unused area.
    */
    void*
    data() noexcept
    {
        return begin_;
    }

    /** Return the size of the unused area.
    */
    std::size_t
    size() const noexcept
    {
        return head_ - begin_;
    }

    /** Clear the contents while preserving capacity.
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Convert unused storage to reserved storage.

        @throws std::invalid_argument n >= this->size()
    */
    BOOST_HTTP_PROTO_DECL
    void
    reserve(std::size_t n);

    template<class T>
    auto
    push(T&& t) ->
        typename std::decay<T>::type&;

    template<class T>
    T*
    push_array(
        std::size_t n,
        T const& t);

private:
    BOOST_HTTP_PROTO_DECL
    void*
    bump_down(
        std::size_t size,
        std::size_t align);
};

template<class T>
auto
workspace::
push(T&& t) ->
    typename std::decay<T>::type&
{
    struct alignas(alignof(::max_align_t))
        U : any
    {
        typename std::decay<T>::type v_;

        U() = delete;
        U(U&&) = default;

        explicit U(T&& t)
            : v_(std::move(t))
        {
        }
    };

    auto p = ::new(bump_down(
        sizeof(U), alignof(U))) U(
            std::forward<T>(t));
    p->next = reinterpret_cast<
        any*>(head_);
    head_ = reinterpret_cast<
        unsigned char*>(p);
    return p->v_;
}

template<class T>
T*
workspace::
push_array(
    std::size_t n,
    T const& t)
{
    struct alignas(alignof(::max_align_t))
        U : any
    {
        std::size_t n_ = 0;

        U() = default;
        ~U()
        {
            for(std::size_t i = n_;
                    i-- > 0;)
                data()[i].~T();
        }

        U(  std::size_t n,
            T const& t)
            : U()
        {
            while(n_ < n)
            {
                new(&data()[n_]) T(t);
                ++n_;
            }
        }

        T* data() noexcept
        {
            return reinterpret_cast<
                T*>(this + 1);
        }
    };

    auto p = ::new(bump_down(
        sizeof(U) + n * sizeof(T),
            alignof(::max_align_t))) U(n, t);
    p->next = reinterpret_cast<
        any*>(head_);
    head_ = reinterpret_cast<
        unsigned char*>(p);
    return p->data();
}

} // detail
} // http_proto
} // boost

#endif
