//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/buffers/range.hpp>
#include <utility>

namespace boost {
namespace http_proto {

class serializer::
    const_buffers_type
{
    std::size_t n_ = 0;
    buffers::const_buffer const* p_ = nullptr;

    friend class serializer;

    const_buffers_type(
        buffers::const_buffer const* p,
        std::size_t n) noexcept
        : n_(n)
        , p_(p)
    {
    }

public:
    using iterator = buffers::const_buffer const*;
    using const_iterator = iterator;
    using value_type = buffers::const_buffer;
    using reference = buffers::const_buffer;
    using const_reference = buffers::const_buffer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    const_buffers_type() = default;
    const_buffers_type(
        const_buffers_type const&) = default;
    const_buffers_type& operator=(
        const_buffers_type const&) = default;

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
    class ConstBufferSequence,
    class>
void
serializer::
start(
    message_view_base const& m,
    ConstBufferSequence&& body)   
{
    start_init(m);
    auto const& bs =
        ws_.push(std::forward<
            ConstBufferSequence>(body));
    std::size_t n = std::distance(
        buffers::begin(bs),
        buffers::end(bs));
    buf_ = make_array(n);
    auto p = buf_.data();
    for(buffers::const_buffer b :
            buffers::range(bs))
        *p++ = b;
    start_buffers(m);
}

template<
    class Source,
    class... Args,
    class>
Source&
serializer::
start(
    message_view_base const& m,
    Args&&... args)
{
    static_assert(
        std::is_constructible<Source, Args...>::value ||
        std::is_constructible<Source, buffered_base::allocator&, Args...>::value,
        "The Source cannot be constructed with the given arguments");

    start_init(m);
    auto& src = construct_source<Source>(
        std::forward<Args>(args)...);
    start_source(m, std::addressof(src));
    return src;
}

//------------------------------------------------

inline
auto
serializer::
make_array(std::size_t n) ->
    detail::array_of_const_buffers 
{
    return {
        ws_.push_array(n,
        buffers::const_buffer{}),
        n };
}

} // http_proto
} // boost

#endif
