//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_PARAMETER_HPP
#define BOOST_HTTP_PROTO_RFC_PARAMETER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/rfc/quoted_token_view.hpp>

namespace boost {
namespace http_proto {

/** An HTTP header parameter

    @par BNF
    @code
    parameter   = token "=" ( token / quoted-string )
    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7231#section-3.1.1.1"
        >3.1.1.1.  Media Type (rfc7231)</a>
*/
struct parameter
{
    string_view name;
    quoted_token_view value;
};

//------------------------------------------------

/** Rule matching parameter

    @par Value Type
    @code
    using value_type = parameter;
    @endcode

    @par Example
    @code
    @endcode

    @par BNF
    @code
    parameter   = token "=" ( token / quoted-string )
    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7231#section-3.1.1.1"
        >3.1.1.1.  Media Type (rfc7231)</a>

    @see
        @ref quoted_token_view
*/
#ifdef BOOST_HTTP_PROTO_DOCS
constexpr __implementation_defined__ parameter_rule;
#else
struct parameter_rule_t
{
    using value_type = parameter;

    BOOST_HTTP_PROTO_DECL
    auto
    parse(
        char const*&,
        char const*) const noexcept ->
            result<value_type>;
};

constexpr parameter_rule_t parameter_rule{};
#endif

} // http_proto
} // boost

#endif
