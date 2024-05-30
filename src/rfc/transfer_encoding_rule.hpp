//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_TRANSFER_ENCODING_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_TRANSFER_ENCODING_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/rfc/list_rule.hpp>
#include <boost/http_proto/rfc/quoted_token_rule.hpp>
#include <boost/url/grammar/range_rule.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {
namespace detail {

//------------------------------------------------

/** A value of Transfer-Encoding
*/
struct transfer_encoding
{
    enum coding
    {
        unknown = 0,
        chunked,
        compress,
        deflate,
        gzip
    };

    struct param
    {
        core::string_view key;
        quoted_token_view value;
    };

    coding id = unknown;
    core::string_view str;
    grammar::range<param> params;
};

//------------------------------------------------

/** Rule to match transfer-coding

    @par Value Type
    @code
    using value_type = transfer_encoding;
    @endcode

    @par Example
    @code
    @endcode

    @par BNF
    @code
    transfer-coding    = "chunked"
                        / "compress"
                        / "deflate"
                        / "gzip"
                        / transfer-extension
    transfer-extension = token *( OWS ";" OWS transfer-parameter )
    transfer-parameter = token BWS "=" BWS ( token / quoted-string )
    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7230#section-3.3.1"
        >3.3.1.  Transfer-Encoding (rfc7230)</a>
*/
#ifdef BOOST_HTTP_PROTO_DOCS
constexpr __implementation_defined__ transfer_encoding_rule;
#else
struct transfer_encoding_rule_t
{
    using value_type = transfer_encoding;

    BOOST_HTTP_PROTO_DECL
    auto
    parse(
        char const*& it,
        char const* end) const noexcept ->
            system::result<value_type>;
};

constexpr transfer_encoding_rule_t transfer_encoding_rule_impl{};
#endif

//------------------------------------------------

/** Rule matching the Transfer-Encoding field value

    @par Value Type
    @code
    using value_type = grammar::range< transfer_encoding >;
    @endcode

    @par Example
    @code
    @endcode

    @par BNF
    @code
    Transfer-Encoding  = 1#transfer-coding
    @endcode

    @par Specification
    @li <a href="https://www.rfc-editor.org/rfc/rfc7230#section-3.3.1"
        >3.3.1.  Transfer-Encoding (rfc7230)</a>
*/
constexpr auto transfer_encoding_rule =
    list_rule( transfer_encoding_rule_impl, 1 );

} // detail
} // http_proto
} // boost

#endif
