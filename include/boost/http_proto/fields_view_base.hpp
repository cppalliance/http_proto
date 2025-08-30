//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
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

#include <boost/core/detail/string_view.hpp>
#include <boost/url/grammar/recycled.hpp>
#include <boost/url/grammar/type_traits.hpp>

#include <iosfwd>
#include <string>

namespace boost {
namespace http_proto {

/** A read-only, bidirectional range of HTTP fields.

    This is a mix-in used to add common
    functionality to derived classes.
*/
class fields_view_base
{
    detail::header const* ph_;

    friend class fields;
    template<std::size_t>
    friend class static_fields;
    friend class fields_base;
    friend class fields_view;
    friend class message_base;
    friend class message_view_base;
    friend class request_base;
    friend class request;
    template<std::size_t>
    friend class static_request;
    friend class request_view;
    friend class response_base;
    friend class response;
    template<std::size_t>
    friend class static_response;
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

    /** A view to an HTTP field.

        The view will be invalidated when the
        underlying container is modified.

        The caller is responsible for ensuring
        that the lifetime of the container extends
        until it is no longer referenced.
    */
    struct reference
    {
        /** Field name constant.

            Set to `boost::none` if the constant
            does not exist in @ref field.
        */
        boost::optional<field> const id;

        /// A view to the field name.
        core::string_view const name;
        
        /// A view to the field value.
        core::string_view const value;

        reference const*
        operator->() const noexcept
        {
            return this;
        }
    };

    /// @copydoc reference
    typedef reference const_reference;

    /** A value type which represent an HTTP field.

        This type allows for making a copy of
        a field where ownership is retained
        in the copy.
    */
    struct value_type
    {
        /** Field name constant.

            Set to `boost::none` if the
            constant does not exist in @ref field.
        */
        boost::optional<field> id;

        /// Field name.
        std::string name;

        /// Field value.
        std::string value;

        /// Constructor.
        BOOST_HTTP_PROTO_DECL
        value_type(
            reference const& other);

        /** Conversion.

            @see
                @ref reference.

            @return A view to the fields.
        */
        operator reference() const noexcept;
    };

    /** A bidirectional iterator to HTTP fields.
    */
    class iterator;

    /// @copydoc iterator
    using const_iterator = iterator;

    /** A bidirectional reverse iterator to HTTP fields.
    */
    class reverse_iterator;

    /// @copydoc iterator
    using const_reverse_iterator = reverse_iterator;

    /** A forward range of matching fields.

        Objects of this type are returned by
        the function @ref find_all.
    */
    class subrange;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the largest possible serialized message.
    */
    static
    constexpr
    std::size_t
    max_size() noexcept
    {
        // TODO: this doesn't take into account
        // the start-line
        return detail::header::max_offset;
    }

    /** Return an iterator to the beginning.
    */
    iterator
    begin() const noexcept;

    /** Return an iterator to the end.
    */
    iterator
    end() const noexcept;

    /** Return a reverse iterator to the beginning.
    */
    reverse_iterator
    rbegin() const noexcept;

    /** Return a reverse iterator to the end.
    */
    reverse_iterator
    rend() const noexcept;

    /** Return a string view representing the serialized data.
    */
    core::string_view
    buffer() const noexcept
    {
        return core::string_view(
            ph_->cbuf, ph_->size);
    }

    /** Return the number of fields in the container.
    */
    std::size_t
    size() const noexcept
    {
        return ph_->count;
    }

    /** Return the value of a field, or throws an exception.

        If more than one field with the specified
        name exists, the first field defined by
        insertion order is returned.

        @par Exception Safety
        Strong guarantee.

        @throw std::out_of_range
        Field is not found.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    at(field id) const;

    /** Return the value of a field, or throws an exception.

        If more than one field with the specified
        name exists, the first field defined by
        insertion order is returned.

        If `name` refers to a known field, it is
        faster to call @ref at with a field id
        instead of a string.

        @par Exception Safety
        Strong guarantee.

        @throw std::out_of_range
        Field is not found.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    at(core::string_view name) const;

    /** Return true if a field exists.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    exists(field id) const noexcept;

    /** Return true if a field exists.

        If `name` refers to a known field,
        it is faster to call @ref exists
        with a field id instead of a string.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    bool
    exists(
        core::string_view name) const noexcept;

    /** Return the number of matching fields.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(field id) const noexcept;

    /** Return the number of matching fields.

        If `name` refers to a known field,
        it is faster to call @ref count
        with a field id instead of a string.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(
        core::string_view name) const noexcept;

    /** Return an iterator to the matching element if it exists.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(field id) const noexcept;

    /** Return an iterator to the matching element if it exists.

        If `name` refers to a known field,
        it is faster to call @ref find
        with a field id instead of a string.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        core::string_view name) const noexcept;

    /** Return an iterator to the matching element if it exists.

        @param from The position to begin the
        search from. This can be `end()`.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from,
        field id) const noexcept;

    /** Return an iterator to the matching element if it exists.

        If `name` refers to a known field,
        it is faster to call @ref find
        with a field id instead of a string.

        @param from The position to begin the
        search from. This can be `end()`.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from,
        core::string_view name) const noexcept;

    /** Return an iterator to the matching element if it exists.

        @param before One past the position
        to begin the search from. This can
        be `end()`.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find_last(
        iterator before,
        field id) const noexcept;

    /** Return an iterator to the matching element if it exists.

        If `name` refers to a known field,
        it is faster to call @ref find_last
        with a field id instead of a string.

        @param before One past the position
        to begin the search from. This can
        be `end()`.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find_last(
        iterator before,
        core::string_view name) const noexcept;

    /** Return the value of a field or a default if missing.

        @param id The field name constant.

        @param s The value to be returned if
        field does not exist.
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    value_or(
        field id,
        core::string_view s) const noexcept;

    /** Return the value of a field or a default if missing.

        If `name` refers to a known field,
        it is faster to call @ref value_or
        with a field id instead of a string.

        @param name The field name.

        @param s The value to be returned if
        field does not exist.
    */
    BOOST_HTTP_PROTO_DECL
    core::string_view
    value_or(
        core::string_view name,
        core::string_view s) const noexcept;

    /** Return a forward range containing values for all matching fields.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(field id) const noexcept;

    /** Return a forward range containing values for all matching fields.

        If `name` refers to a known field,
        it is faster to call @ref find_all
        with a field id instead of a string.

        @param name The field name.
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(
        core::string_view name) const noexcept;
};

/** Format the container to the output stream

    This function serializes the container to
    the specified output stream.

    @par Example
    @code
    request req;
    std::stringstream ss;
    ss << req;
    assert( ss.str() == "GET / HTTP/1.1\r\n\r\n" );
    @endcode

    @par Effects
    @code
    return os << f.buffer();
    @endcode

    @par Complexity
    Linear in `f.buffer().size()`

    @par Exception Safety
    Basic guarantee.

    @return A reference to the output stream, for chaining

    @param os The output stream to write to.

    @param f The container to write.
*/
BOOST_HTTP_PROTO_DECL
std::ostream&
operator<<(
    std::ostream& os,
    const fields_view_base& f);

} // http_proto
} // boost

#include <boost/http_proto/impl/fields_view_base.hpp>

#endif
