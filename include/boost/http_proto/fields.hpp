//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_HPP
#define BOOST_HTTP_PROTO_FIELDS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class fields_view;
#endif

/** A modifiable container of HTTP fields

    @par Iterators

    Iterators obtained from @ref fields
    containers are not invalidated when
    the underlying container is modified.
*/
class BOOST_SYMBOL_VISIBLE
    fields
    : public fields_base
{
public:
    /** Constructor

        Default-constructed fields have no
        name-value pairs.
    */
    BOOST_HTTP_PROTO_DECL
    fields() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    fields(fields&& other) noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    fields(fields const& other);

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    fields(fields_view_base const& f);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields&
    operator=(fields&& f) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields&
    operator=(fields const& f);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields&
    operator=(fields_view const& f);

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Clear the contents, but not the capacity
    */
    void
    clear() noexcept
    {
        this->fields_base::clear();
    }

    /** Swap this with another instance
    */
    void
    swap(fields& other) noexcept
    {
        this->fields_base::swap(other);
    }

    /** Swap two instances
    */
    friend
    void
    swap(
        fields& v1,
        fields& v2) noexcept
    {
        v1.swap(v2);
    }
};

} // http_proto
} // boost

#endif
