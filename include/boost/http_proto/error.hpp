//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_ERROR_HPP
#define BOOST_HTTP_PROTO_ERROR_HPP

#include <boost/http_proto/detail/config.hpp>

namespace boost {
namespace http_proto {

/// Error codes returned from HTTP algorithms and operations.
enum class error
{
    // VFALCO 3 space indent or
    // else Doxygen malfunctions

    success = 0,

    //
    // Partial success
    //

    /** Serialization paused for Expect
    */
   expect_100_continue,

    /** The message is complete
    */
   end_of_message,

    /** The end of input was reached
    */
   end_of_stream,

    /** The in_place buffer is full
    */
   in_place_overflow,

    /** Additional data is required
    */
   need_data,

    //--------------------------------------------
    //
    // Syntax errors (unrecoverable)
    //

    /// Invalid Connection field value
   bad_connection,

    /// Invalid Content-Length field value or values
   bad_content_length,

    /// Invalid Expect header
   bad_expect,

    /// Syntax error in field-name
   bad_field_name,


    /// Syntax error in field-value
   bad_field_value,

    /// Expected LF after CR
   bad_line_ending,

    /// Syntax error in list
   bad_list,

    /// Syntax error in method
   bad_method,

    /// Syntax error in number
   bad_number,

    /// Something wrong with payload fields
   bad_payload,

    /// Syntax error in HTTP-Version
   bad_version,

    /// Syntax error in reason-phrase
   bad_reason,

    /// Syntax error in request-target
   bad_request_target,

    /// Syntax error in status-code
   bad_status_code,

    /// Syntax error in status-line
   bad_status_line,

    /// Syntax error in transfer-encoding
   bad_transfer_encoding,

    /// Syntax error or illegal Upgrade
   bad_upgrade,

    //
    // Bad request / Bad response
    //

    /** Body too large

        The parser encountered a message
        body whose size which exceeded the
        configured limit.
    */
   body_too_large,

    /** Headers too large.
     *
        The combined size of the start line and
        the header fields exceeded the maximum
        configured size.
    */
   headers_limit,

    /** Start-line too large.
     *
        The size of the start line exceeded the
        maximum configured size.
    */
   start_line_limit,

    /** Field too large.

        The size of an individual field exceeded
        the maximum configured size.
    */
   field_size_limit,

    /** Too many fields.

        The number of fields in the message
        exceeded the maximum configured number
        of allowed fields.
    */
   fields_limit,

    /** The message is incomplete

        The end of the stream was encountered
        before the message could be completed.
    */
   incomplete,

    //
    // Metadata errors
    //

    /// A number overflowed
   numeric_overflow,

    /**
     *  Multiple Content-Length fields present

        This error indicates there are
        two or more Content-Length headers
        with different field values.
    */
   multiple_content_length,

    //
    // Other errors
    //

    /**
     *  A dynamic buffer's maximum size would be exceeded
    */
   buffer_overflow
};

// VFALCO we need a bad_message condition?

/** Error conditions corresponding to sets of library error codes.
*/
enum class condition
{
    /** More input data is required.
    */
    need_more_input
};

} // http_proto
} // boost

#include <boost/http_proto/impl/error.hpp>

#endif
