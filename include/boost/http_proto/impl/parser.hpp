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

//------------------------------------------------

template<class DynamicBuffer, class>
typename std::decay<
    DynamicBuffer>::type&
parser::
set_body(
    DynamicBuffer&& b)
{
    // body must not be set already
    if(how_ != how::in_place)
        detail::throw_logic_error();

    // headers must be complete
    if(! got_header())
        detail::throw_logic_error();

    auto& dyn = ws_.push(
        buffers::any_dynamic_buffer_impl<typename
            std::decay<DynamicBuffer>::type,
                buffers_N>(std::forward<
                    DynamicBuffer>(b)));
    dyn_ = &dyn;
    how_ = how::dynamic;
    on_set_body();
    return dyn.buffer();
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

    auto& s = ws_.push(
        std::forward<Sink>(sink));
    sink_ = &s;
    how_ = how::sink;
    on_set_body();
    return s;
}

} // http_proto
} // boost

#endif
