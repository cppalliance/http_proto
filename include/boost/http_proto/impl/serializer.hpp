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

class serializer::buffers
{
    std::size_t n_ = 0;
    const_buffer const* p_ = nullptr;

    friend class serializer;

    buffers(
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

    buffers() = default;
    buffers(
        buffers const&) = default;
    buffers& operator=(
        buffers const&) = default;

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
set_body(Body&& body)
{
    // can't set body twice
    BOOST_ASSERT(! src_);
    BOOST_ASSERT(cbn_ == 0);

    set_body_impl(
        std::forward<Body>(body),
        std::is_base_of<
            source, Body>{});
}

template<class Body>
void
serializer::
set_body_impl(
    Body&& body,
    std::true_type)
{
    src_ = &ws_.push(
        std::forward<Body>(body));
    cb_ = ws_.push_array(
        3, const_buffer{});
}

template<
    class Buffers>
void
serializer::
set_body_impl(
    Buffers&& bs0,
    std::false_type)
{
    auto n = std::distance(
        bs0.begin(), bs0.end());
    auto& bs = ws_.push(
        std::move(bs0));
    cb_ = ws_.push_array(
        1 + n, const_buffer{});
    cbn_ = 0;
    cbi_ = 0;
    for(const_buffer b : bs)
        cb_[++cbn_] = b;
}

} // http_proto
} // boost

#endif
