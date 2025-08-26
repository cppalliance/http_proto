//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_PARSER_HPP
#define BOOST_HTTP_PROTO_REQUEST_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/request_view.hpp>

namespace boost {
namespace http_proto {

/// @copydoc parser
/// @brief A parser for HTTP/1 requests.
/// @see @ref response_parser.
class request_parser
    : public parser
{
public:
    /** Configuration settings for request_parser.

        @see
            @ref install_parser_service,
            @ref request_parser.
    */
    struct config : config_base
    {
        /** Constructor
        */
        config() noexcept
        {
            body_limit = 64 * 1024;
        }
    };

    /** Constructor.

        Constructs a parser that uses the @ref
        config parameters installed on the
        provided `ctx`.

        The parser will attempt to allocate
        the required space on startup, with the
        amount depending on the @ref config
        parameters, and will not perform any
        further allocations, except for Brotli
        decoder instances, if enabled.

        Depending on which compression algorithms
        are enabled in the @ref config, the parser
        will attempt to access the corresponding
        decoder services on the same `ctx`.

        @par Example
        @code
        request_parser sr(ctx);
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        Calls to allocate may throw.

        @param ctx Context from which the
        request_parser will access registered
        services. The caller is responsible for
        ensuring that the provided ctx remains
        valid for the lifetime of the request_parser.

        @see
            @ref install_parser_service,
            @ref config.
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    request_parser(const rts::context& ctx);

    /** Constructor.

        The states of `other` are transferred
        to the newly constructed object,
        including the allocated buffer.
        After construction, the only valid
        operations on the moved-from object
        are destruction and assignment.

        Buffer sequences previously obtained
        using @ref prepare or @ref pull_body
        remain valid.

        @par Complexity
        Constant.

        @param other The parser to move from.
    */
    request_parser(
        request_parser&& other) noexcept = default;

    /** Destructor.

        Any views or buffers obtained from this
        parser become invalid.
    */
    ~request_parser() = default;

    /** Return a read-only view to the parsed request headers.

        The returned view remains valid until:
        @li @ref start is called
        @li @ref reset is called
        @li The parser instance is destroyed

        @par Preconditions
        @code
        this->got_header() == true
        @endcode

        @par Exception Safety
        Strong guarantee.

        @see
            @ref got_header.
    */
    BOOST_HTTP_PROTO_DECL
    request_view
    get() const;
};

} // http_proto
} // boost

#endif
