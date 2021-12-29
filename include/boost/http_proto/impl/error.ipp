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
#include <type_traits>

namespace boost {
namespace http_proto {

namespace detail {

struct http_error_category
    : public error_category
{
    const char*
    name() const noexcept override
    {
        return "boost.http_proto";
    }

    std::string
    message(int ev) const override
    {
        switch(static_cast<error>(ev))
        {
        case error::end: return "range end";
        case error::end_of_message: return "end of message";
        case error::end_of_stream: return "end of stream";

        case error::bad_content_length: return "bad content-length";
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
        case error::bad_transfer_encoding: return "bad transfer-encoding";
        case error::syntax: return "syntax error_";

        case error::body_limit: return "body limit";
        case error::header_limit: return "header limit";
        case error::numeric_overflow: return "numeric overflow";

        default:
            return "boost.http_proto error";
        }
    }

    error_condition
    default_error_condition(
        int ev) const noexcept override
    {
        switch(static_cast<error>(ev))
        {
        case error::end:
        case error::end_of_message:
        case error::end_of_stream:
            return condition::partial_success;

        case error::bad_content_length:
        case error::bad_field_name:
        case error::bad_field_value:
        case error::bad_line_ending:
        case error::bad_list:
        case error::bad_method:
        case error::bad_number:
        case error::bad_version:
        case error::bad_reason:
        case error::bad_request_target:
        case error::bad_status_code:
        case error::bad_status_line:
        case error::bad_transfer_encoding:
        case error::syntax:
            return condition::syntax_error;

        case error::body_limit:
        case error::header_limit:
        case error::numeric_overflow:
        default:
            return {ev, *this};
        }
    }
};

struct http_condition_category
    : error_category
{
    const char*
    name() const noexcept override
    {
        return "boost.http_proto";
    }

    std::string
    message(int cv) const override
    {
        switch(static_cast<condition>(cv))
        {
        case condition::partial_success:
            return "partial success";

        default: // not used
        case condition::syntax_error:
            return "syntax error";
        }
    }
};

} // detail

error_code
make_error_code(error ev) noexcept
{
    static detail::http_error_category const cat{};
    return error_code{static_cast<
        std::underlying_type<error>::type>(ev), cat};
}

error_condition
make_error_condition(condition c) noexcept
{
    static detail::http_condition_category const cat{};
    return error_condition{static_cast<
        std::underlying_type<condition>::type>(c), cat};
}

} // http_proto
} // boost

#endif
