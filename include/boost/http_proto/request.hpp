//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_HPP
#define BOOST_HTTP_PROTO_REQUEST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/headers.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/version.hpp>

namespace boost {
namespace http_proto {

/** Container for HTTP requests
*/
class request
{
    http_proto::method method_;
    http_proto::version version_;
    std::size_t method_size_;
    std::size_t target_size_;

public:
    /** The field values for this request
    */
    headers fields;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request();

    BOOST_HTTP_PROTO_DECL
    request(request&&) noexcept;

    BOOST_HTTP_PROTO_DECL
    request(request const&);

    //
    // Observers
    //

    /** Return the serialized string of this request
    */
    string_view
    str() const noexcept
    {
        return fields.str_impl();
    }

    /** Return the method of this request as a known-method enum

        If the method returned is equal to
        @ref method::unknown, the method may
        be obtained as a string instead, by
        calling @ref method_str.
    */
    http_proto::method
    method() const noexcept
    {
        return method_;
    }

    /** Return the method of this request as a string
    */
    string_view
    method_str() const noexcept
    {
        return string_view(
            fields.str_impl().data(),
            method_size_);
    }

    /** Return the target of this request as a string
    */
    string_view
    target() const noexcept
    {
        return string_view(
            fields.str_impl().data() +
                method_size_ + 1,
            target_size_);
    }

    /** Return the HTTP version of this request
    */
    http_proto::version
    version() const noexcept
    {
        return version_;
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Clear the contents, but not the capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Set the method of the request to the enum
    */
    void
    set_method(http_proto::method m)
    {
        set(
            m, to_string(m),
            target(), version());
    }

    /** Set the method of the request to the string
    */
    void
    set_method(string_view s)
    {
        set(
            string_to_method(s), s,
            target(), version());
    }

    /** Set the target string of the request

        This function sets the request-target.
        The caller is responsible for ensuring
        that the string passed is syntactically
        valid.
    */
    void
    set_target(string_view s)
    {
        set(
            method_, method_str(),
            s, version());
    }

    /** Set the HTTP version of the request
    */
    void
    set_version(
        http_proto::version v)
    {
        set(
            method_, method_str(),
            target(), v);
    }

    /** Set the method, target, and version of the request

        This is more efficient than setting the
        properties individually.
    */
    void
    set(http_proto::method m,
        string_view t,
        http_proto::version v)
    {
        set(
            m, to_string(m),
            t, v);
    }

    /** Set the method, target, and version of the request

        This is more efficient than setting the
        properties individually.
    */
    void
    set(string_view m,
        string_view t,
        http_proto::version v)
    {
        set(
            string_to_method(m), m,
            t, v);
    }

private:
    BOOST_HTTP_PROTO_DECL
    void
    set(http_proto::method m,
        string_view ms,
        string_view t,
        http_proto::version v);
};

} // http_proto
} // boost

#endif
