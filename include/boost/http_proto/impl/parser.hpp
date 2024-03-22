//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// we need a pragma once for the circular includes required
// clangd's intellisense
#pragma once

#ifndef BOOST_HTTP_PROTO_IMPL_PARSER_HPP
#define BOOST_HTTP_PROTO_IMPL_PARSER_HPP

#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/sink.hpp>
#include <boost/http_proto/detail/type_traits.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

template<class ElasticBuffer>
typename std::enable_if<
    ! detail::is_reference_wrapper<
        ElasticBuffer>::value &&
    ! is_sink<ElasticBuffer>::value>::type
parser::
set_body(
    ElasticBuffer&& eb)
{
    // If this goes off it means you are trying
    // to pass by lvalue reference. Use std::ref
    // instead.
    static_assert(
        ! std::is_reference<ElasticBuffer>::value,
        "Use std::ref instead of pass-by-reference");

    // Check ElasticBuffer type requirements
    static_assert(
        buffers::is_dynamic_buffer<ElasticBuffer>::value,
        "Type requirements not met.");

    // body must not be set already
    if(how_ != how::in_place)
        detail::throw_logic_error();

    // headers must be complete
    if(! got_header())
        detail::throw_logic_error();

    auto& dyn = ws_.emplace<
        buffers::any_dynamic_buffer_impl<typename
            std::decay<ElasticBuffer>::type,
                buffers_N>>(std::forward<ElasticBuffer>(eb));
    eb_ = &dyn;
    how_ = how::elastic;
    on_set_body();
}

template<class ElasticBuffer>
void
parser::
set_body(
    std::reference_wrapper<ElasticBuffer> eb)
{
    // Check ElasticBuffer type requirements
    static_assert(
        buffers::is_dynamic_buffer<ElasticBuffer>::value,
        "Type requirements not met.");

    // body must not be set already
    if(how_ != how::in_place)
        detail::throw_logic_error();

    // headers must be complete
    if(! got_header())
        detail::throw_logic_error();

    auto& dyn = ws_.emplace<
        buffers::any_dynamic_buffer_impl<typename
            std::decay<ElasticBuffer>::type&,
                buffers_N>>(eb);
    eb_ = &dyn;
    how_ = how::elastic;
    on_set_body();
}

//------------------------------------------------

template<class Sink>
typename std::enable_if<
    is_sink<Sink>::value,
    typename std::decay<Sink>::type
        >::type&
parser::
set_body(
    Sink&& sink)
{
    // body must not be set already
    if(how_ != how::in_place)
        detail::throw_logic_error();

    // headers must be complete
    if(! got_header())
        detail::throw_logic_error();

    auto& s = ws_.emplace<Sink>(
        std::forward<Sink>(sink));
    sink_ = &s;
    how_ = how::sink;
    on_set_body();
    return s;
}

} // http_proto
} // boost

#endif
