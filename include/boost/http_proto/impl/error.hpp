//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_ERROR_HPP
#define BOOST_HTTP_PROTO_IMPL_ERROR_HPP

#include <type_traits>

namespace boost {
namespace system {
template<>
struct is_error_code_enum<
    ::boost::http_proto::error>
{
    static bool const value = true;
};
} // system
} // boost

namespace boost {
namespace http_proto {

BOOST_HTTP_PROTO_DECL
error_code
make_error_code(error ev) noexcept;

} // http_proto
} // boost

#endif
