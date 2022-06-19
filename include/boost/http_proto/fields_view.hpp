//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
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
        detail::header const& h) noexcept
        : fields_view_base(h)
    {
        BOOST_ASSERT(h.kind ==
            detail::kind::fields);
    }

public:
    /** Constructor

        Default constructed field views
        have a zero size.
    */
    fields_view() noexcept
        : fields_view_base(
            detail::kind::fields)
    {
    }

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    fields_view(
        fields_view const&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields_view&
    operator=(
        fields_view const&) noexcept;

    //--------------------------------------------

    /** Swap this with another instance
    */
    BOOST_HTTP_PROTO_DECL
    void
    swap(fields_view& other) noexcept;

    /** Swap two instances
    */
    friend
    void
    swap(
        fields_view& v1,
        fields_view& v2) noexcept
    {
        v1.swap(v2);
    }
};

} // http_proto
} // boost

#endif
