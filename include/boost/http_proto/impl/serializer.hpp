//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP

#include <boost/http_proto/detail/except.hpp>
#include <iterator>
#include <new>
#include <utility>

namespace boost {
namespace http_proto {

class serializer::
    output_buffers
{
    std::size_t n_ = 0;
    const_buffer const* p_ = nullptr;

    friend class serializer;

    output_buffers(
        const_buffer const* p,
        std::size_t n) noexcept
        : n_(n)
        , p_(p)
    {
    }

public:
    using iterator = const_buffer const*;
    using const_iterator = iterator;
    using value_type = const_buffer;
    using reference = const_buffer;
    using const_reference = const_buffer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    output_buffers() = default;
    output_buffers(
        output_buffers const&) = default;
    output_buffers& operator=(
        output_buffers const&) = default;

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

//------------------------------------------------

template<class Body>
void
serializer::
reset(
    message_view_base const& m,
    Body&& body)
{
    using T = typename
        std::decay<Body>::type;

    // If you get an error here it means
    // that your body type is not derived
    // from source and is not a buffer
    // sequence.
    BOOST_STATIC_ASSERT(
        is_const_buffers<T>::value ||
        std::is_convertible<
            T*, source*>::value);

    ws_.clear();
    reset_impl(
        m,
        std::forward<Body>(body),
        std::integral_constant<
            bool,
            std::is_convertible<
                T*, source const*
                    >::value>{});
}

template<class Source>
void
serializer::
reset_impl(
    message_view_base const& m,
    Source&& source,
    std::true_type)
{
    bp_ = nullptr;
    src_ = std::addressof(
        ws_.push(std::forward<
            Source>(source)));
    reset_impl(m);
}

template<class Buffers>
void
serializer::
reset_impl(
    message_view_base const& m,
    Buffers&& buffers,
    std::false_type)
{
    src_ = nullptr;
    auto& bs = ws_.push(
        make_buffers(std::forward<
            Buffers>(buffers)));
    auto n = std::distance(
        bs.begin(), bs.end());
    bp_ = ws_.push_array(
        n, const_buffer{});
    bn_ = 0;
    for(const_buffer b : bs)
        bp_[bn_++] = b;
    reset_impl(m);
}

} // http_proto
} // boost

#endif
