//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_TOKEN_HPP
#define BOOST_HTTP_PROTO_BNF_TOKEN_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for token

    @par BNF
    @code
    token             = 1*tchar
    @endcode
*/
class token
{
    string_view s_;

public:
    using value_type = string_view;

    string_view const&
    value() const noexcept
    {
        return s_;
    }

    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
        char const* const start,
        char const* const end,
        error_code& ec);
};

} // bnf
} // http_proto
} // boost

#endif
