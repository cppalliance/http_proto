//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_NUMBER_HPP
#define BOOST_HTTP_PROTO_BNF_NUMBER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** BNF for 1*DIGIT
*/
struct number_bnf
{
    std::uint64_t value;

    BOOST_HTTP_PROTO_DECL
    char const*
    parse_element(
        char const* const start,
        char const* const end,
        error_code& ec);
};

} // http_proto
} // boost

#endif
