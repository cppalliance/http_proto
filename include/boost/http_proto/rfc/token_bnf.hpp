//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_TOKEN_BNF_HPP
#define BOOST_HTTP_PROTO_RFC_TOKEN_BNF_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/url/error.hpp>
#include <boost/url/bnf/charset.hpp>

namespace boost {
namespace http_proto {

template<class CharSet>
string_view
parse_token(
    char const*& it,
    char const* end,
    CharSet const& cs) noexcept
{
    auto start = it;
    it = urls::bnf::find_if_not(
        it, end, cs);
    return string_view(
        start, it - start);
}

struct token_bnf
{
    string_view t;

    string_view
    operator*() const noexcept
    {
        return t;
    }

    friend
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        token_bnf& t)
    {
        t.t = parse_token(
            it, end, tchars);
        if(t.t.empty())
        {
            ec = urls::error::syntax;
            return false;
        }
        return true;
    }
};

} // http_proto
} // boost

#endif
