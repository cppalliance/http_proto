//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_VIEW_BASE_HPP
#define BOOST_HTTP_PROTO_FIELDS_VIEW_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/url/grammar/recycled.hpp>
#include <boost/url/grammar/type_traits.hpp>
#include <boost/core/detail/string_view.hpp>
#include <iterator>
#include <memory>
#include <string>

namespace boost {
namespace http_proto {

/** A read-only, bidirectional range of HTTP fields

    This is a mix-in used to add common
    functionality to derived classes.
*/
class fields_view_base
{
    detail::header const* ph_;

    friend class fields;
    friend class fields_base;
    friend class fields_view;
    friend class message_base;
    friend class message_view_base;
    friend class request;
    friend class request_view;
    friend class response;
    friend class response_view;
    friend class serializer;

    explicit
    fields_view_base(
        detail::header const* ph) noexcept
        : ph_(ph)
    {
    }

    fields_view_base(
        fields_view_base const&) = default;
    fields_view_base&
        operator=(fields_view_base const&) = default;

public:
    //--------------------------------------------
    //
    // Types
    //
    //--------------------------------------------

    /** A field
    */
    /**@{*/
    struct reference
    {
        field const id;
        core::string_view const name;
        core::string_view const value;

    #ifndef BOOST_HTTP_PROTO_DOCS
        reference const*
        operator->() const noexcept
        {
            return this;
        }
    #endif
    };

    typedef reference const_reference;
    /**@}*/

    /** A type which can represent a field as a value

        This type allows for making a copy of
        a field where ownership is retained
        in the copy.
    */
    struct value_type
    {
        field id;
        std::string name;
        std::string value;

        BOOST_HTTP_PROTO_DECL
        value_type(
            reference const& other);

        operator reference() const noexcept;
    };

    /** An unsigned integer type
    */
    using size_type = std::size_t;

    /** A signed integer type
    */
    using difference_type =
        std::ptrdiff_t;

    /** A bidirectional iterator to HTTP fields
    */
    /**@{*/
#ifdef BOOST_HTTP_PROTO_DOCS
    using iterator = __see_below__;
#else
    class iterator;
#endif

    using const_iterator = iterator;
    /**@}*/

    /** A bidirectional reverse iterator to HTTP fields
    */
    /**@{*/
#ifdef BOOST_HTTP_PROTO_DOCS
    using reverse_iterator = __see_below__;
#else
    class reverse_iterator;
#endif

    using const_reverse_iterator = reverse_iterator;
    /**@}*/

    /** A forward range of matching fields

        Objects of this type are returned by
        the function @ref find_all.
    */
#ifdef BOOST_HTTP_PROTO_DOCS
    using subrange = __see_below__;
#else
    class subrange;
#endif

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Returns the largest possible serialized message
    */
    static
    constexpr
    std::size_t
    max_size() noexcept
    {
        return max_offset;
    }

    /** Return an iterator to the beginning
    */
    iterator
    begin() const noexcept;

    /** Return an iterator to the end
    */
    iterator
    end() const noexcept;

    /** Return a reverse iterator to the beginning
    */
    reverse_iterator
    rbegin() const noexcept;

    /** Return a reverse iterator to the end
    */
    reverse_iterator
    rend() const noexcept;

    //---

    /** Return a string representing the serialized data
    */
    core::string_view
    buffer() const noexcept
    {
        return core::string_view(
            ph_->cbuf, ph_->size);
    }

    /** Returns the number of fields in the container
    */
    std::size_t
    size() const noexcept
    {
        return ph_->count;
    }

    /** Return true if a field exists
    */
    BOOST_HTTP_PROTO_DECL
    bool
    exists(field id) const noexcept;

    /** Return true if a field exists
    */
    BOOST_HTTP_PROTO_DECL
    bool
    exists(
        core::string_view name) const noexcept;

    /** Return the number of matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(field id) const noexcept;

    /** Return the number of matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(
        core::string_view name) const noexcept;

    /** Returns an iterator to the matching element if it exists
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(field id) const noexcept;

    /** Returns an iterator to the matching element if it exists

        If `name` refers to a known field, it is faster
        to call @ref find with a field id instead of a
        string.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        core::string_view name) const noexcept;

    /** Returns an iterator to the matching element if it exists
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from,
        field id) const noexcept;

    /** Returns an iterator to the matching element if it exists
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from,
        core::string_view name) const noexcept;

    /** Returns an iterator to the matching element if it exists
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find_last(
        iterator before,
        field id) const noexcept;

    /** Returns an iterator to the matching element if it exists
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find_last(
        iterator before,
        core::string_view name) const noexcept;

    /** Return the value of a field
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    value_or(
        field id,
        core::string_view s) const noexcept;

    /** Return the value of a field
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    value_or(
        core::string_view name,
        core::string_view s) const noexcept;

    //---

    /** Return a forward range containing values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(field id) const noexcept;

    /** Return a forward range containing values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(
        core::string_view name) const noexcept;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/fields_view_base.hpp>

#endif
