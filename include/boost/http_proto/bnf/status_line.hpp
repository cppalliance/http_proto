//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_STATUS_LINE_HPP
#define BOOST_HTTP_PROTO_BNF_STATUS_LINE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for status-line

    @par BNF
    @code
    status-line     = HTTP-version SP status-code SP reason-phrase CRLF

    HTTP-version    = "HTTP/" DIGIT "." DIGIT
    status-code     = 3DIGIT
    reason-phrase   = *( HTAB / SP / VCHAR / obs-text )

    @endcode

    @see
        @ref version
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.1.2
*/
class status_line
{
public:
    struct value_type
    {
        char version; // 2 digits
        short status_code;
        string_view reason;
    };

    value_type const&
    value() const noexcept
    {
        return v_;
    }

    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec);

private:
    value_type v_;
};

} // bnf
} // http_proto
} // boost

#endif
