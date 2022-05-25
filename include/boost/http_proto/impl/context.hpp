//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CONTEXT_HPP
#define BOOST_HTTP_PROTO_IMPL_CONTEXT_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/type_traits/detected_or.hpp>
#include <utility>

namespace boost {
namespace http_proto {

namespace detail {

template<class T>
using get_key_impl =
    typename T::key_type;

template<class T>
using get_key_type =
    boost::detected_or<T, get_key_impl, T>;

} // detail

//------------------------------------------------

template<class T, class... Args>
T&
make_service(
    context& ctx,
    Args&&... args)
{
    auto const ti = detail::get_type_index<
        detail::get_key_type<T>>();
    auto const ps =
        ctx.find_service_impl(ti);
    if(ps)
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return static_cast<T&>(
        ctx.make_service_impl(ti,
            std::unique_ptr<context::service>(
                new T(ctx, std::forward<
                    Args>(args)...))));
}

template<class T>
T*
find_service(context& ctx) noexcept
{
    auto const ti = detail::get_type_index<
        detail::get_key_type<T>>();
    auto const ps =
        ctx.find_service_impl(ti);
    if(! ps)
        return nullptr;
    return static_cast<T*>(ps);
}

template<class T>
T&
get_service(context& ctx)
{
    auto ps = find_service<T>();
    if(! ps)
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return static_cast<T&>(*ps);
}

} // http_proto
} // boost

#endif
