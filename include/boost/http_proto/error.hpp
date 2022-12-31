//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_ERROR_HPP
#define BOOST_HTTP_PROTO_ERROR_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_types.hpp>

namespace boost {
namespace http_proto {

/// Error codes returned from HTTP algorithms and operations.
enum class error
{
    // VFALCO 3 space indent or
    // else Doxygen malfunctions

    success = 0

    //
    // Partial success
    //

    /** Serialization paused for Expect
    */
    ,expect_100_continue

    /** The message is complete
    */
    ,end_of_message

    /** The end of input was reached
    */
    ,end_of_stream

    //
    // Syntax errors (unrecoverable)
    //

    /// Invalid Connection field value
    ,bad_connection

    /// Invalid Content-Length field value or values
    ,bad_content_length

    /// Invalid Expect header
    ,bad_expect

    /// Syntax error in field-name
    ,bad_field_name

    /// Syntax error in field-value
    ,bad_field_value

    /// Expected LF after CR
    ,bad_line_ending

    /// Syntax error in list
    ,bad_list

    /// Syntax error in method
    ,bad_method

    /// Syntax error in number
    ,bad_number

    /// Syntax error in HTTP-Version
    ,bad_version

    /// Syntax error in reason-phrase
    ,bad_reason

    /// Syntax error in request-target
    ,bad_request_target

    /// Syntax error in status-code
    ,bad_status_code

    /// Syntax error in status-line
    ,bad_status_line

    /// Syntax error in transfer-encoding
    ,bad_transfer_encoding

    /// Syntax error or illegal Upgrade
    ,bad_upgrade

    /// Unspecified syntax error
    ,syntax

    //
    // Other errors (unrecoverable)
    //

    /** Body too large

        The parser encountered a message
        body whose size which exceeded the
        configured limit.
    */
    ,body_too_large

    /** Field too large

        The parser encountered a field whose
        size exceeded the configured limit.
    */
    ,field_too_large

    /** Header too large

        The parser encountered a header whose
        total size exceeded the configured limit.
    */
    ,header_too_large

    /** Too many fields

        The parser encountered a header
        whose total number of fields exceeded
        the configured limit.
    */
    ,too_many_fields

    /// A number overflowed
    ,numeric_overflow

    /**
     *  Multiple Content-Length fields present

        This error indicates there are
        two or more Content-Length headers
        with different field values.
    */
    ,multiple_content_length
};

} // http_proto
} // boost

//--------------------------------------

namespace boost {
namespace system {

template<>
struct is_error_code_enum<
    ::boost::http_proto::error>
{
    static bool const value = true;
};

} // system
} // boost

#include <boost/http_proto/impl/error.hpp>

#endif
