//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_STATUS_CODE_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_STATUS_CODE_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/grammar/parse_tag.hpp>

namespace boost {
namespace http_proto {

/** Rule for status-code

    @par BNF
    @code
    status-code     = 3DIGIT
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.1.2"
        >3.1.2. Status Line (rfc7230)</a>
*/
struct status_code_rule
{
    unsigned v;
    status st;
    string_view s;

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        status_code_rule& t) noexcept
    {
        parse(it, end, ec, t);
    }

private:
    BOOST_HTTP_PROTO_DECL
    static
    void
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        status_code_rule& t) noexcept;
};

} // http_proto
} // boost

#endif
