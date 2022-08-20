//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_VIEW_BASE_HPP
#define BOOST_HTTP_PROTO_FIELDS_VIEW_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/url/grammar/recycled.hpp>
#include <boost/url/grammar/type_traits.hpp>
#include <memory>
#include <string>

namespace boost {
namespace http_proto {

/** A read-only, bidirectional range of HTTP fields

    This is a mix-in used to add common
    functionality to derived classes.
*/
class BOOST_SYMBOL_VISIBLE
    fields_view_base
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
        field id;
        string_view name;
        string_view value;

    #ifndef BOOST_HTTP_PROTO_DOCS
        reference const*
        operator->() const noexcept
        {
            return this;
        }
    #endif
    };

    using const_reference = reference;
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
    // Iterators
    //
    //--------------------------------------------

    /** Return an iterator to the beginning
    */
    iterator
    begin() const noexcept;

    /** Return an iterator to the end
    */
    iterator
    end() const noexcept;

    /** Return the value of a field, or the empty string
    */
    string_view const
    operator[](field id) const noexcept;

    /** Return the value of a field, or the empty string
    */
    string_view const
    operator[](string_view name) const noexcept;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return a string representing the serialized data
    */
    string_view
    string() const noexcept
    {
        return string_view(
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
    exists(string_view name) const noexcept;

    /** Return the number of matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(field id) const noexcept;

    /** Return the number of matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(string_view name) const noexcept;

    /** Return the value of the first matching field if it exists, otherwise throw
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    at(field id) const;

    /** Return the value of the first matching field if it exists, otherwise throw
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    at(string_view name) const;

    /** Return the value of the first matching field, otherwise return the given string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        field id,
        string_view v) const noexcept;

    /** Return the value of the first matching field, otherwise returns the given string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        string_view name,
        string_view v) const noexcept;

    /// Returns an iterator to the first matching field, otherwise returns end()
    BOOST_HTTP_PROTO_DECL
    iterator
    find(field id) const noexcept;

    /** Returns an iterator to the first matching field, otherwise returns end()

        If `name` refers to a known field, it is faster
        to call @ref find with a field id instead of a
        string.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(string_view name) const noexcept;

    /** Search [from, end), from==end is valid
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from,
        field id) const noexcept;

    /** Search for name, starting at from
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from, 
        string_view name) const noexcept;

    /** Return a forward range containing values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(field id) const noexcept;

    /** Return a forward range containing values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(string_view name) const noexcept;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/fields_view_base.hpp>

#endif
