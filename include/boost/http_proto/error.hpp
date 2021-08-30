//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
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
#ifndef BOOST_HTTO_PROTO_DOCS
    success = 0 // VFALCO Is this correct?
#endif

    //
    // Partial success
    //

    /** A BNF list reached the end of its range

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

    /** A parser needs more input to make progress

        This error is returned when the full input
        exactly matches a prefix of the BNF, and
        more input is needed to complete the match.
    */
    ,need_more

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

    /** End of input was reached before the message completed
    */
    ,incomplete

    /// A number overflowed
    ,numeric_overflow

    //
    //
    //

#if 0
    /** The end of the stream was reached.

        This error is returned when attempting to read HTTP data,
        and the stream returns the error `net::error::eof`
        before any octets corresponding to a new HTTP message have
        been received.
    */
    end_of_stream = 1,

    /** The incoming message is incomplete.

        This happens when the end of stream is reached during
        parsing and some octets have been received, but not the
        entire message.
    */
    partial_message,

    /** Additional buffers are required.

        This error is returned during parsing when additional
        octets are needed. The caller should append more data
        to the existing buffer and retry the parse operaetion.
    */
    need_more,

    /** An unexpected body was encountered during parsing.

        This error is returned when attempting to parse body
        octets into a message container which has the
        @ref empty_body body type.

        @see empty_body
    */
    unexpected_body,

    /** Additional buffers are required.

        This error is returned under the following conditions:

        @li During serialization when using @ref buffer_body.
        The caller should update the body to point to a new
        buffer or indicate that there are no more octets in
        the body.

        @li During parsing when using @ref buffer_body.
        The caller should update the body to point to a new
        storage area to receive additional body octets.
    */
    need_buffer,

    /** The end of a chunk was reached
    */
    end_of_chunk,

    /** Buffer maximum exceeded.

        This error is returned when reading HTTP content
        into a dynamic buffer, and the operation would
        exceed the maximum size of the buffer.
    */
    buffer_overflow,

    //
    // (parser errors)
    //

    /// The method is invalid.
    bad_method,

    /// The request-target is invalid.
    bad_target,

    /// The status-code is invalid.
    bad_status_code,

    /// The reason-phrase is invalid.
    bad_reason,

    /// The field name is invalid.
    bad_field,

    /// The field value is invalid.
    bad_value,

    /// The Content-Length is invalid.
    bad_content_length,

    /// The Transfer-Encoding is invalid.
    bad_transfer_encoding,

    /// The chunk syntax is invalid.
    bad_chunk,

    /// The chunk extension is invalid.
    bad_chunk_extension,

    /// An obs-fold exceeded an internal limit.
    bad_obs_fold,

    /** The message body is shorter than expected.

        This error is returned by @ref file_body when an unexpected
        unexpected end-of-file condition is encountered while trying
        to read from the file.
    */
    short_read

,eof
,end_of_body
#endif
};

/** Error conditions corresponding to HTTP errors
*/
enum class condition
{
    /// A recoverable, partial success
    partial_success = 1

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
