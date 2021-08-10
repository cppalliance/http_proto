//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
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
        case error::need_more: return "need more";

        case error::bad_method: return "bad method";
        case error::bad_field_name: return "bad field name";
        case error::bad_field_value: return "bad field value";
        case error::bad_line_ending: return "bad line ending";
        case error::bad_list: return "bad list";
        case error::bad_version: return "bad version";
        case error::bad_request_target: return "bad request-target";
        case error::bad_transfer_encoding: return "bad transfer-encoding";
        case error::bad_content_length: return "bad content-length";

        case error::header_too_large: return "header too large";
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
        case error::need_more:
            return condition::partial_success;

        case error::bad_content_length:
        case error::bad_method:
        case error::bad_field_name:
        case error::bad_field_value:
        case error::bad_line_ending:
        case error::bad_list:
        case error::bad_version:
        case error::bad_request_target:
        case error::bad_transfer_encoding:
            return condition::syntax_error;

        case error::header_too_large:
        case error::numeric_overflow:
        default:
            return {ev, *this};
        }
    }

#if 0
    bool
    equivalent(int ev,
        error_condition const& condition
            ) const noexcept override
    {
        return condition.value() == ev &&
            &condition.category() == this;
    }

    bool
    equivalent(error_code const& error,
        int ev) const noexcept override
    {
        return error.value() == ev &&
            &error.category() == this;
    }
#endif
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
