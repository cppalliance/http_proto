//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP
#define BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view.hpp>

namespace boost {
namespace http_proto {

/** A reference to an HTTP response header
*/
class BOOST_SYMBOL_VISIBLE
    response_view
    : public fields_view_base
{
#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif

    friend class response;
    friend class response_parser;

    explicit
    response_view(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
        BOOST_ASSERT(ph_->kind ==
            detail::kind::response);
    }

public:
    /** Constructor
    */
    response_view() noexcept
        : fields_view_base(
            detail::header::get_default(
                detail::kind::response))
    {
    }

    /** Constructor
    */
    response_view(
        response_view const&) noexcept = default;

    /** Assignment
    */
    response_view&
    operator=(
        response_view const&) noexcept = default;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the reason string

        This field is obsolete in HTTP/1
        and should only be used for display
        purposes.
    */
    string_view
    reason() const noexcept
    {
        return string_view(
            ph_->cbuf + 13,
            ph_->prefix - 15);
    }

    /** Return the status code
    */
    http_proto::status
    status() const noexcept
    {
        return ph_->res.status;
    }

    /** Return the status code integer
    */
    unsigned short
    status_int() const noexcept
    {
        return ph_->res.status_int;
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
    swap(response_view& other) noexcept
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
        response_view& t0,
        response_view& t1) noexcept
    {
        t0.swap(t1);
    }
};

} // http_proto
} // boost

#endif
