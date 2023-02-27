//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_PARAMETER_IPP
#define BOOST_HTTP_PROTO_RFC_IMPL_PARAMETER_IPP

#include <boost/http_proto/rfc/parameter.hpp>
#include <boost/url/grammar/parse.hpp>

namespace boost {
namespace http_proto {

auto
parameter_rule_t::
parse(
    char const*& it,
    char const* end) const noexcept ->
        system::result<value_type>
{
    (void)it;
    (void)end;
    return system::error_code{};
}

} // http_proto
} // boost

#endif
