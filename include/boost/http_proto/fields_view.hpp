//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_VIEW_HPP
#define BOOST_HTTP_PROTO_FIELDS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

/** A read-only, forward range of HTTP fields
*/
class BOOST_SYMBOL_VISIBLE
    fields_view
    : public fields_view_base
{
    friend class fields;

#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif

    explicit
    fields_view(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
        BOOST_ASSERT(ph_->kind ==
            detail::kind::fields);
    }

public:
    /** Constructor

        Default constructed field views
        have a zero size.
    */
    fields_view() noexcept
        : fields_view_base(
            detail::header::get_default(
                detail::kind::fields))
    {
    }

    /** Constructor
    */
    fields_view(
        fields_view const&) noexcept = default;

    /** Assignment
    */
    fields_view&
    operator=(
        fields_view const&) noexcept = default;

    //--------------------------------------------

    /** Swap this with another instance
    */
    void
    swap(fields_view& other) noexcept
    {
        auto ph = ph_;
        ph_ = other.ph_;
        other.ph_ = ph;
    }

    /** Swap two instances
    */
    friend
    void
    swap(
        fields_view& t0,
        fields_view& t1) noexcept
    {
        t0.swap(t1);
    }
};

} // http_proto
} // boost

#endif
