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
    ws_.clear();
    using T = typename
        std::remove_reference<Body>::type;
    reset_impl(
        m,
        std::forward<Body>(body),
        std::integral_constant<
            bool,
            std::is_convertible<
                T const*,
                source const*>::value>{});
}

template<class Source>
void
serializer::
reset_impl(
    message_view_base const& m,
    Source&& source,
    std::true_type)
{
    src_ = std::addressof(
        ws_.push(std::forward<
            Source>(source)));
    cbn_ = 3;
    cb_ = ws_.push_array(
        cbn_, const_buffer{});
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
    auto& bs = ws_.push(
        std::forward<Buffers>(
            buffers));
    auto n = std::distance(
        bs.begin(), bs.end());
    cb_ = ws_.push_array(
        1 + n, const_buffer{});
    cbn_ = 0;
    for(const_buffer b : bs)
        cb_[++cbn_] = b;
    src_ = nullptr;
    reset_impl(m);
}

} // http_proto
} // boost

#endif
