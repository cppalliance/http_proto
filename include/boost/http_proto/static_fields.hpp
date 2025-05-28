//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STATIC_FIELDS_HPP
#define BOOST_HTTP_PROTO_STATIC_FIELDS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** A modifiable static container of HTTP fields
*/
template<std::size_t Capacity>
class static_fields final
    : public fields_base
{
    alignas(entry)
    char buf_[Capacity];

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
    static_fields() noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            detail::kind::fields,
            buf_,
            Capacity)
    {
    }

    /** Constructor
    */
    explicit static_fields(
        core::string_view s)
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            detail::kind::fields,
            buf_,
            Capacity,
            s)
    {
    }

    /** Constructor
    */
    static_fields(
        static_fields const& other) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            *other.ph_,
            buf_,
            Capacity)
    {
    }

    /** Constructor
    */
    static_fields(
        fields_view const& other)
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            *other.ph_,
            buf_,
            Capacity)
    {
    }

    /** Assignment
    */
    static_fields&
    operator=(static_fields const& f) noexcept
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Assignment
    */
    static_fields&
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
};

} // http_proto
} // boost

#endif
