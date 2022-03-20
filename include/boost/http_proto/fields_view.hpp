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

namespace boost {
namespace http_proto {

/** A read-only, forward range of HTTP fields
*/
class BOOST_SYMBOL_VISIBLE
    fields_view
    : public fields_view_base
{
#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif
    using ctor_params =
        fields_view_base::ctor_params;

    explicit
    fields_view(
        ctor_params const& init) noexcept
        : fields_view_base(init)
    {
    }

public:
    /** Constructor

        Default constructed field views
        have a zero size.
    */
    fields_view() noexcept
        : fields_view_base(0)
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
