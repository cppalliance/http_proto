//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_ERROR_IPP
#define BOOST_HTTP_PROTO_IMPL_ERROR_IPP

#include <boost/http_proto/error.hpp>
#include <boost/assert.hpp>
#include <type_traits>

namespace boost {
namespace http_proto {

namespace detail {

static
error_category const&
get_error_cat() noexcept
{
    struct codes : error_category
    {
        codes() noexcept
            : error_category(
                0x3663257e7585fbfd)
        {
        }
            
        const char*
        name() const noexcept override
        {
            return "boost.http.proto";
        }

        std::string
        message(int ev) const override
        {
            switch(static_cast<error>(ev))
            {
            case error::end_of_message: return "end of message";
            case error::end_of_stream: return "end of stream";
            case error::bad_connection: return "bad Connection";
            case error::bad_content_length: return "bad Content-Length";
            case error::bad_field_name: return "bad field name";
            case error::bad_field_value: return "bad field value";
            case error::bad_line_ending: return "bad line ending";
            case error::bad_list: return "bad list";
            case error::bad_method: return "bad method";
            case error::bad_number: return "bad number";
            case error::bad_version: return "bad version";
            case error::bad_reason: return "bad reason-phrase";
            case error::bad_request_target: return "bad request-target";
            case error::bad_status_code: return "bad status-code";
            case error::bad_status_line: return "bad status-line";
            case error::bad_transfer_encoding: return "bad Transfer-Encoding";
            case error::bad_upgrade: return "bad Upgrade";
            case error::syntax: return "syntax error";

            case error::body_too_large: return "body too large";
            case error::field_too_large: return "field too large";
            case error::header_too_large: return "header too large";
            case error::too_many_fields: return "too many fields";
            case error::numeric_overflow: return "numeric overflow";

            case error::multiple_content_length: return "multiple Content-Length";
            default:
                return "unknown";
            }
        }
    };
    static codes const t{};
    return t;
}

} // detail

error_code
make_error_code(
    error ev) noexcept
{
    return error_code{static_cast<
        std::underlying_type<
            error>::type>(ev),
        detail::get_error_cat()};
}

} // http_proto
} // boost

#endif
