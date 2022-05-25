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
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace boost {
namespace http_proto {

/// The type of error code used by the library
using error_code = boost::system::error_code;

/// The type of system error thrown by the library
using system_error = boost::system::system_error;

/// The type of error category used by the library
using error_category = boost::system::error_category;

/// The type of error condition used by the library
using error_condition = boost::system::error_condition;

/// Error codes returned from HTTP algorithms and operations.
enum class error
{
    // VFALCO 3 space indent or
    // else Doxygen malfunctions

#ifndef BOOST_HTTO_PROTO_DOCS
    success = 0 // VFALCO Is this correct?
#endif

    //
    // Partial success
    //

    /**
      * A BNF list reached the end of its range

        This error is returned by the `begin` and
        `increment` functions of a BNF list element
        when iteration reaches one past the last
        element of the range.
    */
    ,end

    /** A complete message has been parsed
    */
    ,end_of_message

    /** The end of input was reached
    */
    ,end_of_stream

    //
    // Syntax errors (unrecoverable)
    //

    /// Syntax error in Content-Length
    ,bad_content_length

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

    /// Unspecified syntax error
    ,syntax

    //
    // Other errors (unrecoverable)
    //

    /** Body limit exceeded.

        The parser detected an incoming message body which
        exceeded a configured limit.
    */
    ,body_limit

    /** Header limit exceeded.

        The parser detected an incoming message header which
        exceeded a configured limit.
    */
    ,header_limit

    /// A number overflowed
    ,numeric_overflow

    /** End of input was reached before the message completed
    */
    ,incomplete
};

//------------------------------------------------

/** Error conditions corresponding to HTTP errors
*/
enum class condition
{
    /**
      * A parser requires additional input
    */
    need_more = 1

    /// A recoverable, partial success
    ,partial_success

    /// Inputs are not standards-conforming
    ,syntax_error
};

#ifndef BOOST_HTTP_PROTO_DOCS

BOOST_HTTP_PROTO_DECL
error_code
make_error_code(
    error ev) noexcept;

BOOST_HTTP_PROTO_DECL
error_condition
make_error_condition(
    condition c) noexcept;

#endif

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

template<>
struct is_error_condition_enum<
    ::boost::http_proto::condition>
{
    static bool const value = true;
};

} // system
} // boost

#endif
