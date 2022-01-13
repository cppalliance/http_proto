//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_REQUEST_TARGET_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_REQUEST_TARGET_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/grammar/parse_tag.hpp>

namespace boost {
namespace http_proto {

/** Rule for request-target

    @par BNF
    @code
    request-target  = origin-form /
                      absolute-form /
                      authority-form /
                      asterisk-form
    @endcode
*/
struct request_target_rule
{
    string_view s;

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        request_target_rule& t) noexcept
    {
        return parse(it, end, ec, t);
    }

private:
    BOOST_HTTP_PROTO_DECL
    static
    void
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        request_target_rule& t) noexcept;
};

} // http_proto
} // boost

#endif
