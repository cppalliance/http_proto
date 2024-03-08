//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_BASE_HPP
#define BOOST_HTTP_PROTO_FIELDS_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/system/result.hpp>

namespace boost {
namespace http_proto {

namespace detail {
struct prefix_op;
} // detail

/** Mixin for modifiable HTTP fields

    @par Iterators

    Iterators obtained from @ref fields
    containers are not invalidated when
    the underlying container is modified.

    @note HTTP field names are case-insensitive.
*/
class fields_base
    : public virtual fields_view_base
{
    detail::header h_;

    class op_t;
    using entry =
        detail::header::entry;
    using table =
        detail::header::table;

    friend class fields;
    friend class request;
    friend class response;
    friend class serializer;
    friend class message_base;
    friend struct detail::header;
    friend struct detail::prefix_op;

    BOOST_HTTP_PROTO_DECL
    explicit
    fields_base(
        detail::kind) noexcept;

    BOOST_HTTP_PROTO_DECL
    fields_base(
        detail::kind,
        core::string_view);

    fields_base(detail::header const&);

public:
    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~fields_base();

    //--------------------------------------------
    //
    // Capacity
    //
    //--------------------------------------------

    /** Returns the largest permissible capacity in bytes
    */
    static
    constexpr
    std::size_t
    max_capacity_in_bytes() noexcept
    {
        using T = detail::header::entry;
        return alignof(T) *
            (((max_offset - 2 + sizeof(T) * (
                    max_offset / 4)) +
                alignof(T) - 1) /
            alignof(T));
    }

    /** Returns the total number of bytes allocated by the container
    */
    std::size_t
    capacity_in_bytes() const noexcept
    {
        return h_.cap;
    }

    /** Clear the contents, but not the capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Reserve a minimum capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    reserve_bytes(std::size_t n);

    /** Remove excess capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    shrink_to_fit() noexcept;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Append a header

        This function appends a new header with the
        specified id and value. The value must be
        syntactically valid or else an error is returned.
        Any leading or trailing whitespace in the new value
        is ignored.
        <br/>
        No iterators are invalidated.

        @par Example
        @code
        request req;

        req.append( field::user_agent, "Boost" );
        @endcode

        @par Complexity
        Linear in `to_string( id ).size() + value.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.

        @param id The field name constant,
        which may not be @ref field::unknown.

        @param value A value, which must be semantically
        valid for the message.

        @return The error, if any occurred.
    */
    system::result<void>
    append(
        field id,
        core::string_view value)
    {
        BOOST_ASSERT(
            id != field::unknown);
        return insert_impl(
            id, to_string(id), value, h_.count);
    }

    /** Append a header

        This function appends a new header with the
        specified name and value. Both values must be
        syntactically valid or else an error is returned.
        Any leading or trailing whitespace in the new
        value is ignored.
        <br/>
        No iterators are invalidated.

        @par Example
        @code
        request req;

        req.append( "User-Agent", "Boost" );
        @endcode

        @par Complexity
        Linear in `name.size() + value.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.

        @param name The header name.

        @param value A value, which must be semantically
        valid for the message.

        @return The error, if any occurred.
    */
    system::result<void>
    append(
        core::string_view name,
        core::string_view value)
    {
        return insert_impl(
            string_to_field(
                name),
            name,
            value,
            h_.count);
    }

    /** Insert a header

        If a matching header with the same name
        exists, it is not replaced. Instead, an
        additional header with the same name is
        inserted. Names are not case-sensitive.
        Any leading or trailing whitespace in
        the new value is ignored.
        <br>
        All iterators that are equal to `before`
        or come after are invalidated.

        @par Example
        @code
        request req;

        req.insert( req.begin(), field::user_agent, "Boost" );
        @endcode

        @par Complexity
        Linear in `to_string( id ).size() + value.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.

        @return An iterator the newly inserted header, or
        an error if any occurred.

        @param before Position to insert before.

        @param id The field name constant,
        which may not be @ref field::unknown.

        @param value A value, which must be semantically
        valid for the message.
    */
    system::result<iterator>
    insert(
        iterator before,
        field id,
        core::string_view value)
    {
        // TODO: this should probably return an error
        BOOST_ASSERT(
            id != field::unknown);

        auto rv = insert_impl(
            id, to_string(id), value, before.i_);

        if( rv.has_error() )
            return rv.error();
        return before;
    }

    /** Insert a header

        If a matching header with the same name
        exists, it is not replaced. Instead, an
        additional header with the same name is
        inserted. Names are not case-sensitive.
        Any leading or trailing whitespace in
        the new value is ignored.
        <br>
        All iterators that are equal to `before`
        or come after are invalidated.

        @par Example
        @code
        request req;

        req.insert( req.begin(), "User-Agent", "Boost" );
        @endcode

        @par Complexity
        Linear in `name.size() + value.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.

        @return An iterator the newly inserted header, or
        an error if any occurred.

        @param before Position to insert before.

        @param name The header name.

        @param value A value, which must be semantically
        valid for the message.
    */
    system::result<iterator>
    insert(
        iterator before,
        core::string_view name,
        core::string_view value)
    {
        auto rv = insert_impl(
            string_to_field(
                name),
            name,
            value,
            before.i_);

        if( rv.has_error() )
            return rv.error();
        return before;
    }

    //--------------------------------------------

    /** Erase headers

        This function removes the header pointed
        to by `it`.
        <br>
        All iterators that are equal to `it`
        or come after are invalidated.

        @par Complexity
        Linear in `name.size() + value.size()`.

        @par Exception Safety
        Throws nothing.

        @return An iterator to the inserted
        element.

        @param it An iterator to the element
        to erase.
    */
    iterator
    erase(iterator it) noexcept
    {
        erase_impl(it.i_, it->id);
        return it;
    }

    /** Erase headers

        This removes all headers whose name
        constant is equal to `id`.
        <br>
        If any headers are erased, then all
        iterators equal to or that come after
        the first erased element are invalidated.
        Otherwise, no iterators are invalidated.

        @par Complexity
        Linear in `this->string().size()`.

        @par Exception Safety
        Throws nothing.

        @return The number of headers erased.

        @param id The field name constant,
        which may not be @ref field::unknown.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(field id) noexcept;

    /** Erase all matching fields

        This removes all headers with a matching
        name, using a case-insensitive comparison.
        <br>
        If any headers are erased, then all
        iterators equal to or that come after
        the first erased element are invalidated.
        Otherwise, no iterators are invalidated.

        @par Complexity
        Linear in `this->string().size()`.

        @par Exception Safety
        Throws nothing.

        @return The number of fields erased

        @param name The header name.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(
        core::string_view name) noexcept;

    //--------------------------------------------

    /** Set a header value

        Uses the given value to overwrite the
        current one in the header field pointed to by the
        iterator. The value must be syntactically
        valid or else an error is returned.
        Any leading or trailing whitespace in the new value
        is ignored.

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.

        @return The error, if any occurred.

        @param it An iterator to the header.

        @param value A value, which must be semantically
        valid for the message.
    */
    BOOST_HTTP_PROTO_DECL
    system::result<void>
    set(
        iterator it,
        core::string_view value);

    /** Set a header value

        The container is modified to contain exactly
        one field with the specified id set to the given value,
        which must be syntactically valid or else an error is
        returned.
        Any leading or trailing whitespace in the new value
        is ignored.

        @par Postconditions
        @code
        this->count( id ) == 1 && this->at( id ) == value
        @endcode

        @par Complexity

        @return The error, if any occurred.

        @param id The field constant of the
        header to set.

        @param value A value, which must be semantically
        valid for the message.
    */
    BOOST_HTTP_PROTO_DECL
    system::result<void>
    set(
        field id,
        core::string_view value);

    /** Set a header value

        The container is modified to contain exactly
        one field with the specified name set to the given value,
        which must be syntactically valid or else an error is
        returned.
        Any leading or trailing whitespace in the new value
        is ignored.

        @par Postconditions
        @code
        this->count( name ) == 1 && this->at( name ) == value
        @endcode

        @return The error, if any occurred.

        @param name The field name.

        @param value A value, which must be semantically
        valid for the message.
    */
    BOOST_HTTP_PROTO_DECL
    system::result<void>
    set(
        core::string_view name,
        core::string_view value);

    //--------------------------------------------

private:
    BOOST_HTTP_PROTO_DECL
    void
    copy_impl(
        detail::header const&);

    void
    insert_impl_unchecked(
        field id,
        core::string_view name,
        core::string_view value,
        std::size_t before,
        bool has_obs_fold);

    BOOST_HTTP_PROTO_DECL
    system::result<void>
    insert_impl(
        field id,
        core::string_view name,
        core::string_view value,
        std::size_t before);

    BOOST_HTTP_PROTO_DECL
    void
    erase_impl(
        std::size_t i,
        field id) noexcept;

    void raw_erase(
        std::size_t) noexcept;

    std::size_t
    erase_all_impl(
        std::size_t i0,
        field id) noexcept;

    std::size_t
    offset(
        std::size_t i) const noexcept;

    std::size_t
    length(
        std::size_t i) const noexcept;

    void raw_erase_n(field, std::size_t) noexcept;
};

//------------------------------------------------

#ifndef BOOST_HTTP_PROTO_DOCS
namespace detail {
inline
header&
header::
get(fields_base& f) noexcept
{
    return f.h_;
}
} // detail
#endif

} // http_proto
} // boost

#endif
