//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_CONNECTION_HPP
#define BOOST_HTTP_PROTO_RULE_CONNECTION_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/bnf/list.hpp>
#include <boost/http_proto/bnf/token.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** Rule for Connection header field value

    @par BNF
    @code
    Connection        = 1#connection-option
    connection-option = token
    @encode

    @see
        @ref list
        @ref token
        https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
*/
using connection = list_of_one_or_more<token>;

} // bnf
} // http_proto
} // boost

#endif
