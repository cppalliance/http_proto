//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_OWS_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_OWS_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/url/grammar/error.hpp>

namespace boost {
namespace http_proto {

/** Rule for OWS

    @par BNF
    @code
    OWS         = *( SP / HTAB )
    @endcode
*/
struct ows_rule
{
    friend
    inline
    bool
    parse(
        char const*& it,
        char const* end,
        error_code&,
        ows_rule) noexcept;
};

//------------------------------------------------

bool
parse(
    char const*& it,
    char const* end,
    error_code&,
    ows_rule) noexcept
{
    while(it != end)
    {
        if( *it != ' ' &&
            *it != '\t')
            break;
        ++it;
    }
    return true;
}

} // http_proto
} // boost

#endif
