//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_REASON_PHRASE_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_REASON_PHRASE_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** BNF for reason-phrase

    @par BNF
    @code
    reason-phrase   = *( HTAB / SP / VCHAR / obs-text )
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.1.2"
        >3.1.2. Status Line (rfc7230)</a>

    @see
        @ref status_line_rule.
*/
struct reason_phrase_rule
{
    string_view s;

    BOOST_HTTP_PROTO_DECL
    friend
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        reason_phrase_rule& t) noexcept;
};

} // http_proto
} // boost

#endif
