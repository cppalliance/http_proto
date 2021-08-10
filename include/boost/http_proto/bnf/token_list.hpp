//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_TOKEN_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_TOKEN_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/required_list.hpp>

namespace boost {
namespace http_proto {

/** BNF for token-list

    A token list consists of one or more tokens
    separated by commas and optional whitespace.

    @par BNF
    @code
    token-list        = 1#token
    token             = 1*tchar

    legacy list rules:
    1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-6.1
        https://datatracker.ietf.org/doc/html/rfc7230#section-7
*/
struct token_list_bnf
    : required_list<token_list_bnf>
{
    string_view value;

    char const*
    parse_element(
        char const* const start,
        char const* const end,
        error_code& ec)
    {
        tchar_set ts;
        auto it = ts.skip(start, end);
        if(it == start)
        {
            // missing or invalid token
            ec = error::bad_list;
            return start;
        }
        value = { start, static_cast<
            std::size_t>(it - start) };
        return it;
    }
};

using token_list = range<token_list_bnf>;

} // http_proto
} // boost

#endif
