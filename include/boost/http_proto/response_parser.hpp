//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_PARSER_HPP
#define BOOST_HTTP_PROTO_RESPONSE_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/status.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    response_parser
    : public parser
{
public:
    /** Configuration settings for parsing requests
    */
    struct config : config_base
    {
        /** Constructor
        */
        config() noexcept
        {
            body_limit = 1024 * 1024;
        }
    };

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response_parser(context& ctx);

    /** Prepare for the next message on the stream.

        This informs the parser not to read a
        payload for the next message, regardless
        of the presence or absence of certain
        fields such as Content-Length or a chunked
        Transfer-Encoding. Depending on the request,
        some responses do not carry a body. For
        example, a 200 response to a CONNECT
        request from a tunneling proxy, or a
        response to a HEAD request. In these
        cases, callers may use this function
        inform the parser that no body is
        expected. The parser will consider the
        message complete after the header has
        been received.

        @par Preconditions

        This function must called before any calls to parse
        the current message.

        @see
            https://datatracker.ietf.org/doc/html/rfc7230#section-3.3
    */
    void
    start_head_response()
    {
        start_impl(true);
    }

    /** Return the parsed response headers.
    */
    BOOST_HTTP_PROTO_DECL
    response_view
    get() const;
};

} // http_proto
} // boost

#endif
