//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_HPP
#define BOOST_HTTP_PROTO_FIELDS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/core/detail/string_view.hpp>
#include <initializer_list>

namespace boost {
namespace http_proto {

/** A modifiable container of HTTP fields
*/
class fields final
    : public fields_base
{
public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor

        Default-constructed fields have no
        name-value pairs.
    */
    BOOST_HTTP_PROTO_DECL
    fields() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    fields(
        core::string_view s);

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    fields(
        std::size_t initial_size);

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
    fields(fields_view const& other);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields&
    operator=(fields&& f) noexcept;

    /** Assignment
    */
    fields&
    operator=(fields const& f) noexcept
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Assignment
    */
    fields&
    operator=(fields_view const& f)
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Conversion
    */
    operator fields_view() const noexcept
    {
        return fields_view(ph_);
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Swap this with another instance
    */
    void
    swap(fields& other) noexcept
    {
        h_.swap(other.h_);
    }

    /** Swap two instances
    */
    // hidden friend
    friend
    void
    swap(
        fields& t0,
        fields& t1) noexcept
    {
        t0.swap(t1);
    }
};

} // http_proto
} // boost

#endif
