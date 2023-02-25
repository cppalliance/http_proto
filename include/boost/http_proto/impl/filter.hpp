//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_BUFFERS_IMPL_FILTER_HPP
#define BOOST_BUFFERS_IMPL_FILTER_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/buffers/range.hpp>
#include <boost/buffers/type_traits.hpp>

namespace boost {
namespace http_proto {

/*
namespace detail {

template<
    class Buffers,
    class = void>
struct buffer_traits
{
};

template<class T>
struct buffer_traits<T, void_t<
    typename std::enable_if<
        is_mutable_buffer_sequence<T>::value
            >::type
    > >
{
    using value_type = mutable_buffer;
    using pair_type = mutable_buffer_pair;
    using span_type = mutable_buffer_span;
    using subspan_type = mutable_buffer_subspan;
};

template<class T>
struct buffer_traits<T, void_t<
    typename std::enable_if<
        is_const_buffer_sequence<T>::value &&
        ! is_mutable_buffer_sequence<T>::value
            >::type
    > >
{
    using value_type = const_buffer;
    using pair_type = const_buffer_pair;
    using span_type = const_buffer_span;
    using subspan_type = const_buffer_subspan;
};

} // detail
*/

namespace detail {

template<
    class T,
    std::size_t N>
class unrolled
{
    using value_type = typename
        std::conditional<
            buffers::is_mutable_buffer_sequence<T>::value,
            buffers::mutable_buffer,
            buffers::const_buffer
                >::type;

    using span_type = typename
        std::conditional<
            buffers::is_mutable_buffer_sequence<T>::value,
            buffers::mutable_buffer_span,
            buffers::const_buffer_span
                >::type;
    
    using iter_type = decltype(
        begin(std::declval<T const&>()));
    
    using end_type = decltype(
        end(std::declval<T const&>()));

    value_type b_[N];
    iter_type it_;
    end_type end_;
    std::size_t n_;

public:
    explicit
    unrolled(
        T const& t) noexcept
        : it_(begin(t))
        , end_(end(t))
    {
    }

    bool
    empty() const noexcept
    {
        return n_ == 0;
    }

    span_type
    increment()
    {
        n_ = 0;
        while(n_ < N)
        {
            if(it_ == end_)
                break;
            b_[n_++] = *it_++;
        }
        return span_type(&b_[0], n_);
    }
};

} // detail

template<class B0, class B1>
auto
filter::
process_impl(
    B0 const& out,
    B1 const& in,
    bool more) ->
        results
{
    results rv;
    constexpr int N = 16;
    detail::unrolled<B1, N> u0(in);
    detail::unrolled<B0, N> u1(out);

    return rv;
}

} // http_proto
} // boost

#endif
