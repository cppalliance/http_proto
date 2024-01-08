//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/detail/except.hpp>
#include <boost/system/system_error.hpp>
#include <boost/version.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace boost {
namespace http_proto {
namespace detail {

void
throw_bad_alloc(
    source_location const& loc)
{
    throw_exception(
        std::bad_alloc(), loc);
}

void
throw_invalid_argument(
    source_location const& loc)
{
    throw_exception(
        std::invalid_argument(
            "invalid argument"),
        loc);
}

void
throw_invalid_argument(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::invalid_argument(what), loc);
}

void
throw_length_error(
    source_location const& loc)
{
    throw_exception(
        std::length_error(
            "length error"), loc);
}

void
throw_length_error(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::length_error(what), loc);
}

void
throw_logic_error(
    source_location const& loc)
{
    throw_exception(
        std::logic_error(
            "logic error"),
        loc);
}

void
throw_out_of_range(
    source_location const& loc)
{
    throw_exception(
        std::out_of_range("out of range"), loc);
}

void
throw_runtime_error(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::runtime_error(what), loc);
}

void
throw_system_error(
    system::error_code const& ec,
    source_location const& loc)
{
    throw_exception(
        system::system_error(ec), loc);
}

void
throw_system_error(
    error e,
    source_location const& loc)
{
    throw_exception(
        system::system_error(e), loc);
}

} // detail
} // http_proto
} // boost
