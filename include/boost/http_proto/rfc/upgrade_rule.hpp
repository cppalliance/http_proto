//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_UPGRADE_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_UPGRADE_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/rfc/list_rule.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

/** An Upgrade protocol
*/
struct upgrade_protocol
{
    /** The name of the protocol
    */
    string_view name;

    /** Optional protocol version

        An empty version indicates a
        version is not present.
    */
    string_view version;
};

//------------------------------------------------

/** Rule to match Upgrade protocol

    @par Value Type
    @code
    using value_type = upgrade_protocol;
    @endcode

    @par Example
    @code
    @endcode

    @par BNF
    @code
    protocol         = protocol-name ["/" protocol-version]
    protocol-name    = token
    protocol-version = token
    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7230#section-6.7"
        >6.7.  Upgrade (rfc7230)</a>

    @see
        @ref upgrade_protocol.
*/
#ifdef BOOST_HTTP_PROTO_DOCS
constexpr __implementation_defined__ upgrade_protocol_rule;
#else
struct upgrade_protocol_rule_t
{
    using value_type = upgrade_protocol;

    BOOST_HTTP_PROTO_DECL
    auto
    parse(
        char const*& it,
        char const* end) const noexcept ->
            system::result<value_type>;
};

constexpr upgrade_protocol_rule_t upgrade_protocol_rule{};
#endif

//------------------------------------------------

/** Rule matching the Upgrade field value

    @par Value Type
    @code
    using value_type = grammar::range< upgrade_protocol >;
    @endcode

    @par Example
    @code
    @endcode

    @par BNF
    @code
    Upgrade          = 1#protocol

    protocol         = protocol-name ["/" protocol-version]
    protocol-name    = token
    protocol-version = token
    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7230#section-6.7"
        >6.7.  Upgrade (rfc7230)</a>

    @see
        @ref upgrade_protocol.
*/
constexpr auto upgrade_rule = list_rule( upgrade_protocol_rule, 1 );

} // http_proto
} // boost

#endif
