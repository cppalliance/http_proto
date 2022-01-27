//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_HPP
#define BOOST_HTTP_PROTO_RESPONSE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/basic_header.hpp>
#include <boost/http_proto/headers.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/version.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class response_view;
#endif

/** Container for HTTP requests
*/
class BOOST_SYMBOL_VISIBLE
    response
    : public basic_header
{
    http_proto::version version_;
    status result_;

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

    BOOST_HTTP_PROTO_DECL
    response& operator=(response&&) noexcept;

    BOOST_HTTP_PROTO_DECL
    response& operator=(response const&);

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

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

    /** Return a read-only view to the response
    */
    BOOST_HTTP_PROTO_DECL
    operator
    response_view() const noexcept;

    /** Returns a string representing the serialized response
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    get_const_buffer() const noexcept override;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Clear the contents, leaving capacity unchanged
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    set_result(
        status code,
        http_proto::version http_version,
        string_view reason = {});

    /** Swap this with another instance
    */
    BOOST_HTTP_PROTO_DECL
    void
    swap(response& other) noexcept;

    /** Swap two instances
    */
    friend
    void
    swap(
        response& v1,
        response& v2) noexcept
    {
        v1.swap(v2);
    }
};

} // http_proto
} // boost

#endif
