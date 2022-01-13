//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_TOKEN_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_TOKEN_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/rfc/charsets.hpp>
#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/url/grammar/parse_tag.hpp>

namespace boost {
namespace http_proto {

/** Rule for token

    @par BNF
    @code
    token           = 1*tchar

    tchar           = "!" / "#" / "$" / "%" / "&" / "'"
                    / "*" / "+" / "-" / "." / "^" / "_"
                    / "`" / "|" / "~" / DIGIT / ALPHA
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6"
        >3.2.6. Field Value Components (rfc7230)</a>
*/
struct token
{
    using value_type = string_view;

    string_view s;

    string_view
    operator*() const noexcept
    {
        return s;
    }

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        token& t) noexcept
    {
        parse(it, end, ec, t);
    }

private:
    static
    void
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        token& t) noexcept
    {
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return;
        }

        auto const start = it;

        it = grammar::find_if_not(
            it, end, tchars);

        if(it == start)
        {
            ec = grammar::error::syntax;
            return;
        }

        t.s = string_view(
            start, it - start);
    }
};

} // http_proto
} // boost

#endif
