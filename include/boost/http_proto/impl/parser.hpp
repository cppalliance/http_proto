//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_PARSER_HPP
#define BOOST_HTTP_PROTO_IMPL_PARSER_HPP

#include <cstdlib>

namespace boost {
namespace http_proto {

#if 0
class parser::
    buffers
{
    std::size_t n_ = 0;
    mutable_buffer const* p_ = nullptr;

    friend class parser;

    buffers(
        mutable_buffer const* p,
        std::size_t n) noexcept
        : n_(n)
        , p_(p)
    {
    }

public:
    using iterator = mutable_buffer const*;
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
#endif

//------------------------------------------------

template<class Sink, class>
auto
parser::
set_body(
    Sink&& sink) ->
        typename std::decay<
            Sink>::type
{
    auto& body = ws_.push(
        std::forward<
            Sink>(sink));
    return body;
}

} // http_proto
} // boost

#endif
