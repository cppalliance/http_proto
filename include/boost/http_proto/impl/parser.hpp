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

#include <boost/buffers/mutable_buffer_span.hpp>

#include <cstdlib>

namespace boost {
namespace http_proto {

struct parser::any_dynamic
{
    virtual ~any_dynamic() = default;
    virtual buffers::mutable_buffer_span
        prepare(std::size_t) = 0;
    virtual void commit(std::size_t) = 0;
};

template<class T>
struct parser::any_dynamic_impl
    : any_dynamic
{
    T t_;
    buffers::mutable_buffer b_[
        dynamic_N_];

    template<class U>
    explicit
    any_dynamic_impl(U&& u)
        : t_(std::forward<U>(u))
    {
    }

    ~any_dynamic_impl()
    {
    }

    buffers::mutable_buffer_span
    prepare(std::size_t n)
    {
        std::size_t i = 0;
        for(buffers::mutable_buffer b :
            t_.prepare(n))
        {
            b_[i++] = b;
            if(i == dynamic_N_)
                break;
        }
        return { b_, i };
    }

    void
    commit(std::size_t n)
    {
        t_.commit();
    }
};

//------------------------------------------------

template<class DynamicBuffer>
typename std::enable_if<
    buffers::is_dynamic_buffer<
        DynamicBuffer>::value,
    typename std::decay<
        DynamicBuffer>::type
            >::type
parser::
set_body(
    DynamicBuffer&& b)
{
    // body type already chosen
    if(body_ != body::in_place)
        detail::throw_logic_error();

    auto& rv = ws_.push(
        any_dynamic_impl<typename
            std::decay<DynamicBuffer>::type>(
                std::forward<DynamicBuffer>(b)));
    body_ = body::dynamic;
    dynamic_ = &rv;
    return rv;
}

//------------------------------------------------

template<class Sink>
typename std::enable_if<
    buffers::is_sink<Sink>::value,
    typename std::decay<Sink>::type
        >::type
parser::
set_body(
    Sink&& sink)
{
    // body type already chosen
    if(body_ != body::in_place)
        detail::throw_logic_error();

    auto& rv = ws_.push(
        std::forward<
            Sink>(sink));
    body_ = body::sink;
    sink_ = &rv;
    return rv;
}

} // http_proto
} // boost

#endif
