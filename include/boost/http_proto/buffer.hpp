//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BUFFER_HPP
#define BOOST_HTTP_PROTO_BUFFER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/assert.hpp>
#include <boost/config/workaround.hpp>
#include <boost/type_traits/make_void.hpp>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>

namespace boost {
namespace http_proto {

//------------------------------------------------

/** Determine if T is a possibly-const buffer.
*/
#if BOOST_HTTP_PROTO_DOCS
template<bool isConst, class T>
struct is_buffer
    : std::integral_constant<bool, ...>{};
#else

template<
    bool isConst, class T,
    class = void>
struct is_buffer : std::false_type {};

template<
    bool isConst, class T>
struct is_buffer<
    isConst, T, boost::void_t<decltype(
        std::declval<std::size_t&>() =
            std::declval<T const&>().size()
    ),
    typename std::enable_if<
        std::is_same<
            void*, decltype(
                std::declval<T const&>().data())
            >::value || (
        std::is_same<
            void const*, decltype(
                std::declval<T const&>().data())
            >::value && isConst)
        >::type
    >> : std::is_copy_constructible<T>
{
};

#endif

/** Determine if T is a const buffer.
*/
template<class T>
using is_const_buffer = is_buffer<true, T>;

/** Determine if T is a mutable buffer.
*/
template<class T>
using is_mutable_buffer = is_buffer<false, T>;

//------------------------------------------------

/** Determine if T is a buffer sequence.
*/
#if BOOST_HTTP_PROTO_DOCS
template<bool isConst, class T>
struct is_buffers
    : std::integral_constant<bool, ...>{};
#else

template<
    bool isConst, class T,
    class = void>
struct is_buffers
    : is_buffer<isConst, T>
{
};

template<
    bool isConst, class T>
struct is_buffers<
    isConst, T, boost::void_t<
        typename std::enable_if<
            is_buffer<
                isConst, decltype(
                *std::declval<T const&>().begin())
                    >::value &&
            is_buffer<
                isConst, decltype(
                *std::declval<T const&>().end())
                    >::value
            >::type
    >> : std::is_move_constructible<T>
{
};

#endif

/** Determine if T is a const buffers.
*/
template<class T>
using is_const_buffers = is_buffers<true, T>;

/** Determine if T is a mutable buffers.
*/
template<class T>
using is_mutable_buffers = is_buffers<false, T>;

//------------------------------------------------

class mutable_buffer
{
    void* p_ = nullptr;
    std::size_t n_ = 0;

public:
    mutable_buffer() = default;
    mutable_buffer(
        mutable_buffer const&) = default;
    mutable_buffer& operator=(
        mutable_buffer const&) = default;

    mutable_buffer(
        void* data,
        std::size_t size) noexcept
        : p_(data)
        , n_(size)
    {
    }

    template<
        class MutableBuffer
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename std::enable_if<
            is_mutable_buffer<MutableBuffer
                >::value>::type
#endif
    >
    mutable_buffer(
        MutableBuffer const& b) noexcept
        : p_(b.data())
        , n_(b.size())
    {
    }

    void*
    data() const noexcept
    {
        return p_;
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    mutable_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = static_cast<
                char*>(p_) + n_;
            n_ = 0;
            return *this;
        }
        p_ = static_cast<
            char*>(p_) + n;
        n_ -= n;
        return *this;
    }

    friend
    mutable_buffer
    operator+(
        mutable_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return {
                static_cast<char*>(
                    b.data()) + n,
                b.size() - n };
        return {
            static_cast<char*>(
                b.data()) + b.size(),
            0 };
    }

    friend
    mutable_buffer
    operator+(
        std::size_t n,
        mutable_buffer const& b) noexcept
    {
        if(n < b.size())
            return {
                static_cast<char*>(
                    b.data()) + n,
                b.size() - n };
        return {
            static_cast<char*>(
                b.data()) + b.size(),
            0 };
    }
};

//------------------------------------------------

class const_buffer
{
    void const* p_ = nullptr;
    std::size_t n_ = 0;

public:
    const_buffer() = default;
    const_buffer(
        const_buffer const&) = default;
    const_buffer& operator=(
        const_buffer const&) = default;

    const_buffer(
        void const* data,
        std::size_t size) noexcept
        : p_(data)
        , n_(size)
    {
    }

    template<
        class ConstBuffer
#ifndef BOOST_HTTP_PROTO_DOCS
        ,class = typename std::enable_if<
            is_const_buffer<ConstBuffer
                >::value>::type
#endif
    >
    const_buffer(
        ConstBuffer const& b) noexcept
        : p_(b.data())
        , n_(b.size())
    {
    }

    void const*
    data() const noexcept
    {
        return p_;
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    const_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = static_cast<
                char const*>(p_) + n_;
            n_ = 0;
            return *this;
        }
        p_ = static_cast<
            char const*>(p_) + n;
        n_ -= n;
        return *this;
    }

    friend
    const_buffer
    operator+(
        const_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return {
                static_cast<char const*>(
                    b.data()) + n,
                b.size() - n };
        return {
            static_cast<char const*>(
                b.data()) + b.size(),
            0 };
    }

    friend
    const_buffer
    operator+(
        std::size_t n,
        const_buffer const& b) noexcept
    {
        if(n < b.size())
            return {
                static_cast<char const*>(
                    b.data()) + n,
                b.size() - n };
        return {
            static_cast<char const*>(
                b.data()) + b.size(),
            0 };
    }
};

//------------------------------------------------

/** A MutableBuffer of length 1
*/
class mutable_buffers_1
{
    mutable_buffer mb_;

public:
    using value_type =
        mutable_buffer;

    using const_iterator =
        mutable_buffer const*;

    mutable_buffers_1() = default;

    explicit
    mutable_buffers_1(
        mutable_buffer mb) noexcept
        : mb_(mb)
    {
    }

    mutable_buffers_1&
    operator=(
        mutable_buffers_1 const&) = default;

    const_iterator
    begin() const noexcept
    {
        return &mb_;
    }

    const_iterator
    end() const noexcept
    {
        return &mb_ + 1;
    }
};

/** A ConstBuffer of length 1
*/
class const_buffers_1
{
    const_buffer cb_;

public:
    using value_type =
        const_buffer;

    using const_iterator =
        const_buffer const*;

    const_buffers_1() = default;

    explicit
    const_buffers_1(
        const_buffer cb) noexcept
        : cb_(cb)
    {
    }

    const_buffers_1&
    operator=(const_buffers_1 const&) = default;

    const_iterator
    begin() const noexcept
    {
        return &cb_;
    }

    const_iterator
    end() const noexcept
    {
        return &cb_ + 1;
    }
};

//------------------------------------------------

template<
    class ConstBuffer
#ifndef BOOST_HTTP_PROTO_DOCS
    ,class = typename std::enable_if<
        is_const_buffer<
            ConstBuffer>::value &&
        ! is_mutable_buffer<
            ConstBuffer>::value
                >::type
#endif
>
auto
make_buffers(
    ConstBuffer const& b) ->
        const_buffers_1
{
    return const_buffers_1(
        const_buffer(b));
}

template<
    class MutableBuffer
#ifndef BOOST_HTTP_PROTO_DOCS
    ,class = typename std::enable_if<
        is_mutable_buffer<
            MutableBuffer>::value
                >::type
#endif
>
auto
make_buffers(
    MutableBuffer const& b) ->
        mutable_buffers_1
{
    return mutable_buffers_1(
        mutable_buffer(b));
}

template<
    class Buffers
#ifndef BOOST_HTTP_PROTO_DOCS
    ,class = typename std::enable_if<
        ! is_const_buffer<
            Buffers>::value &&
        is_const_buffers<
            Buffers>::value
                >::type
#endif
>
auto
make_buffers(
    Buffers&& b) ->
        Buffers&&
{
    return std::forward<
        Buffers>(b);
}

//------------------------------------------------

#if BOOST_WORKAROUND(BOOST_MSVC, < 1910)
# pragma warning (push)
# pragma warning (disable: 4521) // multiple copy constructors specified
# pragma warning (disable: 4522) // multiple assignment operators specified
#endif

template<bool isConst>
class buffers_pair
{
public:
    // VFALCO: This type is public otherwise
    //         asio::buffers_iterator won't compile.
    using value_type = typename
        std::conditional<isConst,
            const_buffer,
            mutable_buffer>::type;

    using const_iterator = value_type const*;

    buffers_pair() = default;

#if defined(BOOST_HTTP_PROTO_DOCS) || ( \
        ! BOOST_WORKAROUND(BOOST_MSVC, < 1910))
    buffers_pair(
        buffers_pair const& other) = default;
    buffers_pair& operator=(
        buffers_pair const& other) = default;

#else
    buffers_pair(
        buffers_pair const& other)
        : buffers_pair(
            *other.begin(),
            *(other.begin() + 1))
    {
    }

    buffers_pair&
    operator=(buffers_pair const& other)
    {
        b_[0] = other.b_[0];
        b_[1] = other.b_[1];
        return *this;
    }
#endif

    // const pair construction
    // from mutable mutable pair
    template<
        bool isConst_ = isConst,
        class = typename std::enable_if<
            isConst_>::type>
    buffers_pair(
        buffers_pair<false> const& other)
        : buffers_pair(
            other.b_[0],
            other.b_[1])
    {
    }

    // const pair assignment
    // from mutable mutable pair
    template<
        bool isConst_ = isConst,
        class = typename std::enable_if<
            isConst_>::type>
    buffers_pair&
    operator=(
        buffers_pair<false> const& other)
    {
        b_[0] = other.b_[0];
        b_[1] = other.b_[1];
        return *this;
    }

    buffers_pair(
        value_type b0,
        value_type b1) noexcept
    {
        if(b0.size() > 0)
        {
            b_[0] = b0;
            b_[1] = b1;
        }
        else
        {
            b_[0] = b1;
        }
    }

    const_buffer
    operator[](
        std::size_t i) const noexcept
    {
        BOOST_ASSERT(i < 2);
        return b_[i];
    }

    const_iterator
    begin() const noexcept
    {
        return b_;
    }

    const_iterator
    end() const noexcept
    {
        if(b_[1].size() > 0)
            return &b_[2];
        if(b_[0].size() > 0)
            return &b_[1];
        return b_;
    }

private:
    value_type b_[2];
};

#if BOOST_WORKAROUND(BOOST_MSVC, < 1910)
# pragma warning (pop)
#endif

/** A mutable buffers pair
*/
using mutable_buffers_pair =
    buffers_pair<false>;

/** A const buffers pair
*/
using const_buffers_pair =
    buffers_pair<true>;

//------------------------------------------------

/** Return the total octets in a buffer sequence
*/
template<
    class ConstBuffers
#ifndef BOOST_HTTP_PROTO_DOCS
    , class = typename std::enable_if<
        is_const_buffers<ConstBuffers>::value
    >::type
#endif
>
std::size_t
buffer_size(
    ConstBuffers const& buffers) noexcept
{
    std::size_t n = 0;
    for(const_buffer b
            : make_buffers(buffers))
        n += b.size();
    return n;
}

//------------------------------------------------

/** Copy buffer contents
*/
template<
    class MutableBuffers,
    class ConstBuffers>
std::size_t
buffer_copy(
    MutableBuffers const& to,
    ConstBuffers const& from,
    std::size_t at_most =
        std::size_t(-1)) noexcept
{
    // If you get a compile error here it
    // means that one or both of your types
    // do not meet the requirements.
    static_assert(
        is_mutable_buffers<MutableBuffers>::value &&
        is_const_buffers<ConstBuffers>::value,
        "Type requirements not met");

    std::size_t total = 0;
    std::size_t pos0 = 0;
    std::size_t pos1 = 0;
    auto const& bs0 = (make_buffers)(from);
    auto const& bs1 = (make_buffers)(to);
    auto const end0 = bs0.end();
    auto const end1 = bs1.end();
    auto it0 = bs0.begin();
    auto it1 = bs1.begin();
    while(
        total < at_most &&
        it0 != end0 &&
        it1 != end1)
    {
        const_buffer b0 =
            const_buffer(*it0) + pos0;
        mutable_buffer b1 =
            mutable_buffer(*it1) + pos1;
        std::size_t amount =
        [&]
        {
            std::size_t n = b0.size();
            if( n > b1.size())
                n = b1.size();
            if( n > at_most - total)
                n = at_most - total;
            std::memcpy(
                b1.data(),
                b0.data(),
                n);
            return n;
        }();
        total += amount;
        if(amount == b1.size())
        {
            ++it1;
            pos1 = 0;
        }
        else
        {
            pos1 += amount;
        }
        if(amount == b0.size())
        {
            ++it0;
            pos0 = 0;
        }
        else
        {
            pos0 += amount;
        }
    }
    return total;
}

//------------------------------------------------

// VFALCO GARBAGE
class mutable_buffers
{
    mutable_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

public:
    using value_type = mutable_buffer;

    using iterator = value_type const*;

    mutable_buffers() = default;

    mutable_buffers(
        mutable_buffers const&) = default;

    mutable_buffers& operator=(
        mutable_buffers const&) = default;

    mutable_buffers(
        value_type const* p,
        std::size_t n) noexcept
        : p_(p)
        , n_(n)
    {
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    iterator
    begin() const noexcept
    {
        return p_;
    }

    iterator
    end() const noexcept
    {
        return p_ + n_;
    }
};

} // http_proto
} // boost

#endif
