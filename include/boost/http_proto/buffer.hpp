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
#include <iterator>
#include <type_traits>
#include <utility>

namespace boost {
namespace http_proto {

/** size tag for tag_invoke.
*/
struct size_tag {};

/** prefix tag for tag_invoke.
*/
struct prefix_tag {};

/** suffix tag for tag-invoke.
*/
struct suffix_tag {};

//------------------------------------------------

/** Holds a buffer that can be modified.
*/
class mutable_buffer
{
    unsigned char* p_ = nullptr;
    std::size_t n_ = 0;

public:
    using value_type = mutable_buffer;
    using const_iterator =
        value_type const*;

    mutable_buffer() = default;
    mutable_buffer(
        mutable_buffer const&) = default;
    mutable_buffer& operator=(
        mutable_buffer const&) = default;

    mutable_buffer(
        void* data,
        std::size_t size) noexcept
        : p_(static_cast<
            unsigned char*>(data))
        , n_(size)
    {
    }

#ifndef BOOST_HTTP_PROTO_DOCS
    // conversion to boost::asio::mutable_buffer
    template<
        class T
        , class = typename std::enable_if<
            std::is_constructible<T,
                void*, std::size_t>::value
            && ! std::is_same<T, mutable_buffer>::value
            //&& ! std::is_same<T, const_buffer>::value
        >::type
    >
    operator T() const noexcept
    {
        return T{ data(), size() };
    }
#endif

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

    const_iterator
    begin() const noexcept
    {
        return this;
    }

    const_iterator
    end() const noexcept
    {
        return this + 1;
    }

    /** Remove a prefix from the buffer.
    */
    mutable_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = p_ + n_;
            n_ = 0;
            return *this;
        }
        p_ = p_ + n;
        n_ -= n;
        return *this;
    }

    /** Return the buffer with a prefix removed.
    */
    friend
    mutable_buffer
    operator+(
        mutable_buffer b,
        std::size_t n) noexcept
    {
        return b += n;
    }

    /** Return the buffer with a prefix removed.
    */
    friend
    mutable_buffer
    operator+(
        std::size_t n,
        mutable_buffer b) noexcept
    {
        return b += n;
    }

#ifndef BOOST_HTTP_PROTO_DOCS
    friend
    mutable_buffer
    tag_invoke(
        prefix_tag const&,
        mutable_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return { b.p_, n };
        return b;
    }

    friend
    mutable_buffer
    tag_invoke(
        suffix_tag const&,
        mutable_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return { b.p_ + b.n_ - n, n };
        return b;
    }
#endif
};

//------------------------------------------------

/** Holds a buffer that cannot be modified.
*/
class const_buffer
{
    unsigned char const* p_ = nullptr;
    std::size_t n_ = 0;

public:
    using value_type = const_buffer;
    using const_iterator =
        value_type const*;

    const_buffer() = default;
    const_buffer(
        const_buffer const&) = default;
    const_buffer& operator=(
        const_buffer const&) = default;

    const_buffer(
        void const* data,
        std::size_t size) noexcept
        : p_(static_cast<
            unsigned char const*>(data))
        , n_(size)
    {
    }

    const_buffer(
        mutable_buffer const& b) noexcept
        : p_(static_cast<
            unsigned char const*>(b.data()))
        , n_(b.size())
    {
    }

#ifndef BOOST_HTTP_PROTO_DOCS
    // conversion to boost::asio::const_buffer
    template<
        class T
        , class = typename std::enable_if<
            std::is_constructible<T,
                void const*, std::size_t>::value &&
            ! std::is_same<T, mutable_buffer>::value &&
            ! std::is_same<T, const_buffer>::value
        >::type
    >
    operator T() const noexcept
    {
        return T{ data(), size() };
    }
#endif

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

    const_iterator
    begin() const noexcept
    {
        return this;
    }

    const_iterator
    end() const noexcept
    {
        return this + 1;
    }

    /** Remove a prefix from the buffer.
    */
    const_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = p_ + n_;
            n_ = 0;
            return *this;
        }
        p_ = p_ + n;
        n_ -= n;
        return *this;
    }

    /** Return the buffer with a prefix removed.
    */
    friend
    const_buffer
    operator+(
        const_buffer b,
        std::size_t n) noexcept
    {
        return b += n;
    }

    /** Return the buffer with a prefix removed.
    */
    friend
    const_buffer
    operator+(
        std::size_t n,
        const_buffer b) noexcept
    {
        return b += n;
    }

#ifndef BOOST_HTTP_PROTO_DOCS
    friend
    const_buffer
    tag_invoke(
        prefix_tag const&,
        const_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return { b.p_, n };
        return b;
    }

    friend
    const_buffer
    tag_invoke(
        suffix_tag const&,
        const_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return { b.p_ + b.n_ - n, n };
        return b;
    }
#endif
};

//------------------------------------------------

#ifndef BOOST_HTTP_PROTO_DOCS
namespace detail {

// is bidirectional iterator
template<class T, class = void>
struct is_bidir_iter : std::false_type
{
};

template<class T>
struct is_bidir_iter<T, boost::void_t<decltype(
    // LegacyIterator
    *std::declval<T&>()
    ),
    // LegacyIterator
    typename std::iterator_traits<T>::value_type,
    typename std::iterator_traits<T>::difference_type,
    typename std::iterator_traits<T>::reference,
    typename std::iterator_traits<T>::pointer,
    typename std::iterator_traits<T>::iterator_category,
    typename std::enable_if<
    // LegacyIterator
    std::is_copy_constructible<T>::value &&
    std::is_copy_assignable<T>::value &&
    std::is_destructible<T>::value &&
    std::is_same<T&, decltype(
        ++std::declval<T&>())>::value &&
    // Swappable
    //  VFALCO TODO
    // EqualityComparable
    std::is_convertible<decltype(
        std::declval<T const&>() ==
            std::declval<T const&>()),
        bool>::value &&
    // LegacyInputIterator
    std::is_convertible<typename
        std::iterator_traits<T>::reference, typename
        std::iterator_traits<T>::value_type>::value &&
    std::is_same<typename
        std::iterator_traits<T>::reference,
        decltype(*std::declval<T const&>())>::value &&
    std::is_convertible<decltype(
        std::declval<T const&>() !=
            std::declval<T const&>()),
        bool>::value &&
    std::is_same<T&, decltype(
        ++std::declval<T&>())>::value &&
    // VFALCO (void)r++   (void)++r
    std::is_convertible<decltype(
        *std::declval<T&>()++), typename
        std::iterator_traits<T>::value_type>::value &&
    // LegacyForwardIterator
    std::is_default_constructible<T>::value &&
    std::is_same<T, decltype(
        std::declval<T&>()++)>::value &&
    std::is_same<typename
        std::iterator_traits<T>::reference,
            decltype(*std::declval<T&>()++)
                >::value &&
    // LegacyBidirectionalIterator
    std::is_same<T&, decltype(
        --std::declval<T&>())>::value &&
    std::is_convertible<decltype(
        std::declval<T&>()--),
            T const&>::value &&
    std::is_same<typename
        std::iterator_traits<T>::reference,
        decltype(*std::declval<T&>()--)>::value
    >::type >>
    : std::true_type
{
};

} // detail
#endif

//------------------------------------------------

// https://www.boost.org/doc/libs/1_65_0/doc/html/boost_asio/reference/ConstBufferSequence.html

/** Determine if T is a ConstBuffers.
*/
#if BOOST_HTTP_PROTO_DOCS
template<class T>
struct is_const_buffers
    : std::integral_constant<bool, ...>{};
#else

template<class T, class = void>
struct is_const_buffers : std::false_type
{
};

template<class T>
struct is_const_buffers<T const>
    : is_const_buffers<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_const_buffers<T const&>
    : is_const_buffers<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_const_buffers<T&>
    : is_const_buffers<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_const_buffers<T, boost::void_t<
    typename std::enable_if<
        (std::is_same<const_buffer, typename 
            T::value_type>::value
        || std::is_same<mutable_buffer, typename
            T::value_type>::value
            ) &&
        detail::is_bidir_iter<typename
            T::const_iterator>::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().begin())
                >::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().end())
                >::value && (
        std::is_same<const_buffer, typename
            std::remove_const<typename
                std::iterator_traits<
                    typename T::const_iterator
                        >::value_type>::type
                >::value ||
        std::is_same<mutable_buffer, typename
            std::remove_const<typename
                std::iterator_traits<
                    typename T::const_iterator
                        >::value_type>::type
                >::value)
        >::type
    > > : std::is_move_constructible<T>
{
};

#endif

/** Determine if T is a MutableBuffers.
*/
#if BOOST_HTTP_PROTO_DOCS
template<class T>
struct is_mutable_buffers
    : std::integral_constant<bool, ...>{};
#else

template<class T, class = void>
struct is_mutable_buffers : std::false_type
{
};

template<class T>
struct is_mutable_buffers<T const>
    : is_mutable_buffers<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_mutable_buffers<T const&>
    : is_mutable_buffers<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_mutable_buffers<T&>
    : is_mutable_buffers<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_mutable_buffers<T, boost::void_t<
    typename std::enable_if<
        detail::is_bidir_iter<typename
            T::const_iterator>::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().begin())
                >::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().end())
                >::value &&
        std::is_same<mutable_buffer, typename
            std::remove_const<typename
                std::iterator_traits<
                    typename T::const_iterator
                        >::value_type>::type
                >::value
        >::type
    >> : std::is_move_constructible<T>
{
};

#endif

//------------------------------------------------

/** Determine if T is a DynamicBuffer
*/
#if BOOST_HTTP_PROTO_DOCS
template<class T>
struct is_dynamic_buffer
    : std::integral_constant<bool, ...>{};
#else

template<
    class T,
    class = void>
struct is_dynamic_buffer : std::false_type {};

template<class T>
struct is_dynamic_buffer<
    T, boost::void_t<decltype(
        std::declval<std::size_t&>() =
            std::declval<T const&>().size()
        ,std::declval<std::size_t&>() =
            std::declval<T const&>().max_size()
        ,std::declval<std::size_t&>() =
            std::declval<T const&>().capacity()
        ,std::declval<T&>().commit(
            std::declval<std::size_t>())
        ,std::declval<T&>().consume(
            std::declval<std::size_t>())
    )
    ,typename std::enable_if<
        is_const_buffers<typename
            T::const_buffers_type>::value
        && is_mutable_buffers<typename
            T::mutable_buffers_type>::value
        >::type
    ,typename std::enable_if<
        std::is_same<decltype(
            std::declval<T const&>().data()),
            typename T::const_buffers_type>::value
        && std::is_same<decltype(
            std::declval<T&>().prepare(
                std::declval<std::size_t>())),
            typename T::mutable_buffers_type>::value
        >::type
    > > : std::true_type
{
};

#endif

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
        buffers_pair const& other) noexcept
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
    for(const_buffer b : buffers)
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
        is_mutable_buffers<MutableBuffers>::value,
        "Type requirements not met");
    static_assert(
        is_const_buffers<ConstBuffers>::value,
        "Type requirements not met");

    std::size_t total = 0;
    std::size_t pos0 = 0;
    std::size_t pos1 = 0;
    auto const end0 = from.end();
    auto const end1 = to.end();
    auto it0 = from.begin();
    auto it1 = to.begin();
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

//
// size
//

//
// prefix
//

#ifndef BOOST_HTTP_PROTO_DOCS
template<class Buffers>
void
tag_invoke(
    prefix_tag const&,
    Buffers const&,
    std::size_t) = delete;
#endif

/** Returns the type of a prefix of Buffers
*/
#ifdef BOOST_HTTP_PROTO_DOCS
template<class Buffers>
using prefix_type = __see_below__;
#else
template<class Buffers>
using prefix_type = decltype(
    tag_invoke(
        prefix_tag{},
        std::declval<Buffers const&>(),
        std::size_t{}));
#endif

/** Return a prefix of the buffers.
*/
template<class Buffers>
auto
prefix(
    Buffers const& b,
    std::size_t n) ->
        prefix_type<Buffers>
{
    static_assert(
        is_const_buffers<Buffers>::value,
        "Type requirements not met");

    return tag_invoke(
        prefix_tag{}, b, n);
}

/** Return a prefix of the buffers.
*/
template<class Buffers>
auto
sans_suffix(
    Buffers const& b,
    std::size_t n) ->
        prefix_type<Buffers>
{
    auto const n0 = buffer_size(b);
    if( n > n0)
        n = n0;
    return prefix(b, n0 - n);
}

//
// suffix
//

#ifndef BOOST_HTTP_PROTO_DOCS
template<class Buffers>
void
tag_invoke(
    suffix_tag const&,
    Buffers const&,
    std::size_t) = delete;
#endif

/** Returns the type of a suffix of Buffers.
*/
#ifdef BOOST_HTTP_PROTO_DOCS
template<class Buffers>
using suffix_type = __see_below__;
#else
template<class Buffers>
using suffix_type = decltype(
    tag_invoke(
        suffix_tag{},
        std::declval<Buffers const&>(),
        std::size_t{}));
#endif

/** Return a suffix of the buffers.
*/
template<class Buffers>
auto
suffix(
    Buffers const& b,
    std::size_t n) ->
        suffix_type<Buffers>   
{
    static_assert(
        is_const_buffers<Buffers>::value,
        "Type requirements not met");

    return tag_invoke(
        suffix_tag{}, b, n);
}

/** Return a suffix of the buffers.
*/
template<class Buffers>
auto
sans_prefix(
    Buffers const& b,
    std::size_t n) ->
        suffix_type<Buffers>
{
    static_assert(
        is_const_buffers<Buffers>::value,
        "Type requirements not met");

    auto const n0 = buffer_size(b);
    if( n > n0)
        n = n0;
    return suffix(b, n0 - n);
}

} // http_proto
} // boost

#endif
