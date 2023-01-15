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
    output
{
    std::size_t n_ = 0;
    const_buffer const* p_ = nullptr;

    friend class serializer;

    output(
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

    output() = default;
    output(
        output const&) = default;
    output& operator=(
        output const&) = default;

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

template<
    class P0,
    class... Pn>
serializer::
serializer(
    std::size_t buffer_size,
    P0&& p0,
    Pn&&... pn)
    : serializer(buffer_size)
{
    apply_params(
        std::forward<P0>(p0),
        std::forward<Pn>(pn)...);
}

//------------------------------------------------

template<
    class ConstBuffers,
    class>
void
serializer::
reset(
    message_view_base const& m,
    ConstBuffers&& body)   
{
    ws_.clear();
    auto& bs = ws_.push(
        (make_buffers)(std::forward<
            ConstBuffers>(body)));
    std::size_t const pn =
        1 +
        1 + // chunk header
        std::distance(
            bs.begin(), bs.end()) +
        1; // final chunk
    auto const pp = ws_.push_array(
        pn, const_buffer{});
    auto p = pp + 2;
    for(const_buffer b : bs)
        *p++ = b;
    reset_buffers_impl(m, pp, pn);
}

template<
    class Source,
    class>
auto
serializer::
reset(
    message_view_base const& m,
    Source&& source) ->
        typename std::decay<
            Source>::type&
{
    ws_.clear();
    auto& rv = ws_.push(
        std::forward<
            Source>(source));
    reset_source_impl(
        m, std::addressof(rv));
    return rv;
}

template<class MaybeReserve>
auto
serializer::
reset_stream(
    message_view_base const& m,
    MaybeReserve&& maybe_reserve) ->
        stream
{
    // small hack for type-erasing
    struct Source : source
    {
        MaybeReserve&& f;

        void
        maybe_reserve(
            std::size_t limit,
            reserve_fn const& reserve) override
        {
            f(limit, reserve);
        }

        results
        read(mutable_buffers_pair) override
        {
            return {};
        }
    };
    Source src{ std::forward<
        MaybeReserve>(maybe_reserve) };

    ws_.clear();
    reset_stream_impl(m, src);
    return stream{*this};
}

//------------------------------------------------

inline
void
serializer::
apply_params() noexcept
{
}

template<
    class P0,
    class... Pn>
void
serializer::
apply_params(
    P0&& p0,
    Pn&&... pn)
{
    // If you get an error here it means
    // you passed an unknown parameter type.
    apply_param(std::forward<P0>(p0));

    apply_params(
        std::forward<Pn>(pn)...);
}

} // http_proto
} // boost

#endif
