//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_TRANSFER_ENCODING_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_TRANSFER_ENCODING_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/required_list.hpp>
#include <boost/http_proto/bnf/transfer_param_list.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for the Transfer-Encoding header field value

    @par BNF
    @code
    Transfer-Encoding   = 1#transfer-coding
    transfer-coding     = "chunked"
                        / "compress"
                        / "deflate"
                        / "gzip"
                        / transfer-extension
    transfer-extension  = token transfer-param-list

    @endcode

    @see
        @ref transfer_param_list
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.1
        https://datatracker.ietf.org/doc/html/rfc7230#section-4
*/
class transfer_coding
{
public:
    struct value_type
    {
        string_view name;
        range<transfer_param_list> params;

        value_type const*
        operator->() const noexcept
        {
            return this;
        }
    };

    value_type const&
    value() const noexcept
    {
        return v_;
    }

    BOOST_HTTP_PROTO_DECL
    char const*
    parse(
        char const* const start,
        char const* const end,
        error_code& ec);

private:
    value_type v_;
};

using transfer_encoding =
    required_list<transfer_coding>;

} // bnf
} // http_proto
} // boost

#endif
