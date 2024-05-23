//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BUFFERED_BASE_HPP
#define BOOST_HTTP_PROTO_BUFFERED_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

/** Base class for buffered algorithms

    Algorithms implementing @ref filter,
    @ref source, or @ref sink inherit from
    this common interface.
*/
struct BOOST_HTTP_PROTO_DECL
    buffered_base
{
    /** Allocator for buffered algorithms.
    */
    class allocator;

    /** Destructor.
    */
    virtual
    ~buffered_base();

    /** Initialize the algorithm.

        The derived class must be initialized
        before invoking any other members,
        except destruction.
        The default implementation does nothing.
        The purpose of this function is to
        allow the derived class to optionally
        allocate temporary storage using the
        specified allocator, which could offer
        advantages.
        <br>
        Subclasses are still required to operate
        correctly even when insufficient storage
        is available from the allocator. In this
        case they should simply allocate normally.

        @par Preconditions
        Initialization has not already occurred.

        @param a The allocator to use.
    */
    void
    init(allocator& a)
    {
        on_init(a);
    }

    /** Initialize the algorithm.

        The derived class must be initialized
        before invoking any other members,
        except destruction.
        The default implementation does nothing.
        The purpose of this function is to
        allow the derived class to optionally
        allocate temporary storage using the
        specified allocator, which could offer
        advantages.
        <br>
        Subclasses are still required to operate
        correctly even when insufficient storage
        is available from the allocator. In this
        case they should simply allocate normally.

        @par Preconditions
        Initialization has not already occurred.

        @throws std::invalid_argument `max_size > a.max_size()`

        @param a The allocator to use.

        @param max_size The largest allowed
            total amount of bytes for the
            allocator.
    */
    void
    init(
        allocator& a,
        std::size_t max_size);

protected:
    /** Initialize the algorithm.

        The default implementation does nothing.
        The purpose of this function is to
        allow the derived class to optionally
        allocate temporary storage using the
        specified allocator, which could offer
        advantages.
        <br>
        Subclasses are still required to operate
        correctly even when insufficient storage
        is available from the allocator. In this
        case they should simply allocate normally.

        @par Preconditions
        Initialization has not already occurred.

        @param a The allocator to use.
    */
    virtual
    void
    on_init(allocator& a);
};

//------------------------------------------------

/** Provides memory to buffered algorithms.
*/
class buffered_base::allocator
{
public:
    /** Constructor

        Default constructed objects return
        zero from @ref max_size.
    */
    allocator() = default;

    /** Constructor

        This function constructs an allocator
        which uses the specified contiguous
        buffer. Calls to allocate will return
        parcels of the buffer from either the
        beginning or the end depending on the
        value of `downwards`.

        @par Preconditions
        @code
        p != nullptr || n == 0
        @endcode

        @par Exception Safety
        Throws nothing.

        @param p A pointer to contiguous storage.
            This may be `nullptr` if `n == 0`.

        @param n The number of valid bytes of
            storage pointed to by p. This may
            be zero.

        @param downwards When true, calls to
            allocate will return storage from
            the end of the memory pointed to
            by `p` rather than the beginning.
    */
    allocator(
        void* p,
        std::size_t n,
        bool downwards) noexcept
        : base_(static_cast<
            unsigned char*>(p))
        , size_(n)
        , down_(downwards)
    {
    }

    /** The maximum amount that can be successfully returned from reserve
    */
    std::size_t
    max_size() const noexcept
    {
        return size_;
    }

    /** Return the total number of bytes allocated
    */
    std::size_t
    size_used() const noexcept
    {
        return size_used_;
    }

    /** Return a pointer to at least n bytes of contiguous storage

        Allocated storage will be automatically
        deallocated when the @ref filter,
        @ref sink, or @ref source is destroyed.

        @throws std::invalid_argument `n > max_size()`

        @return A pointer to uninitialized storage.

        @param n The number of bytes.
    */
    BOOST_HTTP_PROTO_DECL
    void*
    allocate(std::size_t n);

private:
    void
    remove(
        std::size_t n) noexcept
    {
        if(down_)
            base_ += n;
        size_ -= n;
    }

    void
    restore(
        std::size_t n) noexcept
    {
        if(down_)
            base_ -= n;
        size_ += n;
    }

    friend struct buffered_base;

    unsigned char* base_ = nullptr;
    std::size_t size_ = 0;
    std::size_t size_used_ = 0;
    bool down_ = false;
};

} // http_proto
} // boost

#endif
