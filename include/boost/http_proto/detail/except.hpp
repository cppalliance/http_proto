//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_EXCEPT_HPP
#define BOOST_HTTP_PROTO_DETAIL_EXCEPT_HPP

#include <boost/http_proto/error.hpp>
#include <boost/assert/source_location.hpp>

namespace boost {
namespace http_proto {
namespace detail {

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_bad_alloc(
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_invalid_argument(
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_invalid_argument(
    char const* what,
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_length_error(
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_length_error(
    char const* what,
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_logic_error(
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_out_of_range(
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_runtime_error(
    char const* what,
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_system_error(
    system::error_code const& ec,
    source_location const& loc = BOOST_CURRENT_LOCATION);

void BOOST_HTTP_PROTO_CORE_DECL BOOST_NORETURN throw_system_error(
    error e,
    source_location const& loc = BOOST_CURRENT_LOCATION);

} // detail
} // http_proto
} // boost

#endif
