//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_TYPE_TRAITS_HPP
#define BOOST_HTTP_PROTO_BNF_TYPE_TRAITS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/type_traits/make_void.hpp>
#include <type_traits>

namespace boost {
namespace http_proto {
namespace bnf {

/** Alias for std::true_type if T satisfies Element

    @par Valid expressions
    @code
        char const*
        T::parse(
            char const* start,
            char const* end,
            error_code& ec);
    @endcode
*/
#if BOOST_HTTP_PROTO_DOCS
template<class T>
using is_element = __see_below__;

#else
template<class T, class = void>
struct is_element : std::false_type {};

template<class T>
struct is_element<T, boost::void_t<decltype(
    std::declval<char const*&>() = std::declval<T&>().parse(
        std::declval<char const*>(),
        std::declval<char const*>(),
        std::declval<error_code&>())
            )>> : std::true_type
{
};

#endif

/** Alias for std::true_type if T satisfies Element

    @par Valid expressions
    @code
        char const*
        T::begin(
            char const* start,
            char const* end,
            error_code& ec);

        char const*
        T::increment(
            char const* start,
            char const* end,
            error_code& ec);
    @endcode
*/
#if BOOST_HTTP_PROTO_DOCS
template<class T>
using is_list = __see_below__;

#else
template<class T, class = void>
struct is_list : std::false_type {};

template<class T>
struct is_list<T, boost::void_t<decltype(
    std::declval<char const*&>() = std::declval<T&>().begin(
        std::declval<char const*>(),
        std::declval<char const*>(),
        std::declval<error_code&>()),
    std::declval<char const*&>() = std::declval<T&>().increment(
        std::declval<char const*>(),
        std::declval<char const*>(),
        std::declval<error_code&>())
            )>> : std::true_type
{
};

#endif

} // bnf
} // http_proto
} // boost

#endif
