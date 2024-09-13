//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_FILTER_HPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_FILTER_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/buffers/range.hpp>
#include <boost/buffers/type_traits.hpp>
#include <boost/core/ignore_unused.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<
    class T,
    std::size_t N>
class filter::unrolled
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

template<
    class MutableBuffers,
    class ConstBuffers>
auto
filter::
process_impl(
    MutableBuffers const& out,
    ConstBuffers const& in,
    bool more) ->
        results
{
    boost::ignore_unused(more);
    results rv;
    constexpr int N = 16;
    unrolled<ConstBuffers, N> u0(in);
    unrolled<MutableBuffers, N> u1(out);

    return rv;
}

} // detail
} // http_proto
} // boost

#endif
