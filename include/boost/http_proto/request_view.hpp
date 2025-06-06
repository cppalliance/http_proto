//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_VIEW_HPP
#define BOOST_HTTP_PROTO_REQUEST_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** A read-only reference to an HTTP request
*/
class request_view
    : public message_view_base
{
    friend class request_base;
    friend class request_parser;

    explicit
    request_view(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
        BOOST_ASSERT(ph_->kind ==
            detail::kind::request);
    }

public:
    /** Constructor
    */
    request_view() noexcept
        : fields_view_base(
            detail::header::get_default(
                detail::kind::request))
    {
    }

    /** Constructor
    */
    request_view(
        request_view const&) noexcept = default;

    /** Assignment
    */
    request_view&
    operator=(
        request_view const&) noexcept = default;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the method as an integral constant

        If the method returned is equal to
        @ref method::unknown, the method may
        be obtained as a string instead, by
        calling @ref method_text.
    */
    http_proto::method
    method() const noexcept
    {
        return ph_->req.method;
    };

    /** Return the method as a string
    */
    core::string_view
    method_text() const noexcept
    {
        return core::string_view(
            ph_->cbuf,
            ph_->req.method_len);
    }

    /** Return the request-target string
    */
    core::string_view
    target() const noexcept
    {
        return core::string_view(
            ph_->cbuf +
                ph_->req.method_len + 1,
            ph_->req.target_len);
    }

    /** Return the HTTP-version
    */
    http_proto::version
    version() const noexcept
    {
        return ph_->version;
    }

    /** Swap this with another instance
    */
    void
    swap(request_view& other) noexcept
    {
        auto ph = ph_;
        ph_ = other.ph_;
        ph_ = ph;
    }

    /** Swap two instances
    */
    // hidden friend
    friend
    void
    swap(
        request_view& t0,
        request_view& t1) noexcept
    {
        t0.swap(t1);
    }
};

} // http_proto
} // boost

#endif
