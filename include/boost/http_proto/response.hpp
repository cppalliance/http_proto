//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_HPP
#define BOOST_HTTP_PROTO_RESPONSE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/headers.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/version.hpp>

namespace boost {
namespace http_proto {

/** Container for HTTP requests
*/
class response
{
    // headers have a maximum size of 2^32-1 chars
    using off_t = std::uint32_t;

    http_proto::version
        version_ = version::http_1_1;
    status result_ = status::ok;

public:
    /** Container holding the response header fields
    */
    headers fields;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response();

    BOOST_HTTP_PROTO_DECL
    response(response&&) noexcept;

    BOOST_HTTP_PROTO_DECL
    response(response const&);

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Returns a string representing the serialized response
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    str() const noexcept;

    /** Return the HTTP-version of this message
    */
    http_proto::version
    version() const noexcept
    {
        return version_;
    }

    /** Return the status enumeration of the response result
    */
    BOOST_HTTP_PROTO_DECL
    status
    result() const noexcept;

    /** Return the status of the response result as an integer
    */
    BOOST_HTTP_PROTO_DECL
    unsigned
    result_int() const noexcept;

    /** Return the obsolete reason phrase of the response
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    reason() const noexcept;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    void
    set_result(
        status code,
        http_proto::version http_version,
        string_view reason = {});
};

} // http_proto
} // boost

#endif
