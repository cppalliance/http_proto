//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_BASE_HPP
#define BOOST_HTTP_PROTO_FIELDS_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** Mixin for modifiable HTTP fields.

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
    std::size_t max_cap_ =
        std::numeric_limits<std::size_t>::max();
    bool external_storage_ = false;

    using entry =
        detail::header::entry;
    using offset_type =
        detail::header::offset_type;
    using table =
        detail::header::table;

    class op_t;
    class prefix_op_t
    {
        fields_base& self_;
        offset_type new_prefix_;
        char* buf_ = nullptr;

    public:
        prefix_op_t(
            fields_base& self,
            std::size_t new_prefix,
            core::string_view* s0 = nullptr,
            core::string_view* s1 = nullptr);

        ~prefix_op_t();
    };

    friend class fields;
    template<std::size_t>
    friend class static_fields;
    friend class request_base;
    friend class request;
    template<std::size_t>
    friend class static_request;
    friend class response_base;
    friend class response;
    template<std::size_t>
    friend class static_response;
    friend class message_base;

    BOOST_HTTP_PROTO_DECL
    explicit
    fields_base(
        detail::kind k) noexcept;

    BOOST_HTTP_PROTO_DECL
    fields_base(
        detail::kind k,
        char* storage,
        std::size_t cap) noexcept;

    BOOST_HTTP_PROTO_DECL
    fields_base(
        detail::kind k,
        core::string_view s);

    BOOST_HTTP_PROTO_DECL
    fields_base(
        detail::kind k,
        char* storage,
        std::size_t cap,
        core::string_view s);

    BOOST_HTTP_PROTO_DECL
    explicit
    fields_base(
        detail::header const& h);

    BOOST_HTTP_PROTO_DECL
    fields_base(
        detail::header const& h,
        char* storage,
        std::size_t cap);

public:
    /** Destructor.
    */
    BOOST_HTTP_PROTO_DECL
    ~fields_base();

    //--------------------------------------------
    //
    // Capacity
    //
    //--------------------------------------------

    /** Return the maximum allowed capacity in bytes.
    */
    std::size_t
    max_capacity_in_bytes() noexcept
    {
        return max_cap_;
    }

    /** Return the total number of bytes allocated by the container.
    */
    std::size_t
    capacity_in_bytes() const noexcept
    {
        return h_.cap;
    }

    /** Clear contents while preserving the capacity.

        In the case of response and request
        containers the start-line also resets to
        default.

        @par Postconditions
        @code
        this->size() == 0
        @endcode

        @par Complexity
        Constant.
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Adjust the capacity without changing the size.

        This function adjusts the capacity
        of the container in bytes, without
        affecting the current contents. Has
        no effect if `n <= this->capacity_in_bytes()`.

        @par Postconditions
        @code
        this->capacity_in_bytes() >= n
        @endcode

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param n The capacity in bytes.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reserve_bytes(std::size_t n);

    /** Set the maximum allowed capacity in bytes.

        Prevents the container from growing beyond
        `n` bytes. Exceeding this limit will throw
        an exception.

        @par Preconditions
        @code
        this->capacity_in_bytes() <= n
        @endcode

        @par Postconditions
        @code
        this->max_capacity_in_bytes() == n
        @endcode

        @par Exception Safety
        Strong guarantee.
        Exception thrown on invalid input.

        @throw std::invalid_argument
        `n < this->capacity_in_bytes()`

        @param n The maximum allowed capacity in bytes.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_max_capacity_in_bytes(std::size_t n);

    /** Remove excess capacity.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
    */
    BOOST_HTTP_PROTO_DECL
    void
    shrink_to_fit();

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Append a header.

        This function appends a new header.
        Existing headers with the same name are
        not changed.

        Any leading or trailing whitespace in the
        value is ignored.

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
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param id The field name constant.

        @param value The value which must be semantically
        valid for the message.
    */
    void
    append(
        field id,
        core::string_view value)
    {
        system::error_code ec;
        append(id, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
    }

    /** Append a header.

        This function appends a new header.
        Existing headers with the same name are
        not changed.

        Any leading or trailing whitespace in the
        value is ignored.

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
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param id The field name constant.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    void
    append(
        field id,
        core::string_view value,
        system::error_code& ec)
    {
        insert_impl(
            id,
            to_string(id),
            value,
            h_.count,
            ec);
    }

    /** Append a header.

        This function appends a new header.
        Existing headers with the same name are
        not changed.

        Any leading or trailing whitespace in the
        value is ignored.

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
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param name The header name.

        @param value The header value, which must
        be semantically valid for the message.
    */
    void
    append(
        core::string_view name,
        core::string_view value)
    {
        system::error_code ec;
        append(name, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
    }

    /** Append a header.

        This function appends a new header.
        Existing headers with the same name are
        not changed.

        Any leading or trailing whitespace in the
        value is ignored.

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
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param name The header name.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    void
    append(
        core::string_view name,
        core::string_view value,
        system::error_code& ec)
    {
        insert_impl(
            string_to_field(name),
            name,
            value,
            h_.count,
            ec);
    }

    /** Insert a header.

        If a matching header with the same name
        exists, it is not replaced. Instead, an
        additional header with the same name is
        inserted. Names are not case-sensitive.
        Any leading or trailing whitespace in
        the new value is ignored.

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
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @return An iterator to the newly inserted header.

        @param before Position to insert before.

        @param id The field name constant.

        @param value The value which must be semantically
        valid for the message.
    */
    iterator
    insert(
        iterator before,
        field id,
        core::string_view value)
    {
        system::error_code ec;
        auto const it = insert(
            before, id, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
        return it;
    }

    /** Insert a header.

        If a matching header with the same name
        exists, it is not replaced. Instead, an
        additional header with the same name is
        inserted. Names are not case-sensitive.

        Any leading or trailing whitespace in
        the new value is ignored.

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
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @return An iterator to the newly inserted header.

        @param before Position to insert before.

        @param id The field name constant.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    iterator
    insert(
        iterator before,
        field id,
        core::string_view value,
        system::error_code& ec)
    {
        insert_impl(
            id,
            to_string(id),
            value,
            before.i_,
            ec);
        return before;
    }

    /** Insert a header.

        If a matching header with the same name
        exists, it is not replaced. Instead, an
        additional header with the same name is
        inserted. Names are not case-sensitive.

        Any leading or trailing whitespace in
        the new value is ignored.

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
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @return An iterator to the newly inserted header.

        @param before Position to insert before.

        @param name The header name.

        @param value The value which must be semantically
        valid for the message.
    */
    iterator
    insert(
        iterator before,
        core::string_view name,
        core::string_view value)
    {
        system::error_code ec;
        insert(before, name, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
        return before;
    }

    /** Insert a header.

        If a matching header with the same name
        exists, it is not replaced. Instead, an
        additional header with the same name is
        inserted. Names are not case-sensitive.

        Any leading or trailing whitespace in
        the new value is ignored.

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
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @return An iterator to the newly inserted header.

        @param before Position to insert before.

        @param name The header name.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    iterator
    insert(
        iterator before,
        core::string_view name,
        core::string_view value,
        system::error_code& ec)
    {
        insert_impl(
            string_to_field(name),
            name,
            value,
            before.i_,
            ec);
        return before;
    }

    //--------------------------------------------

    /** Erase headers.

        This function removes the header pointed
        to by `it`.

        All iterators that are equal to `it`
        or come after are invalidated.

        @par Complexity
        Linear in `name.size() + value.size()`.

        @return An iterator to one past the
        removed element.

        @param it The iterator to the element
        to erase.
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    erase(iterator it) noexcept;

    /** Erase headers.

        This removes all headers whose name
        constant is equal to `id`.

        If any headers are erased, then all
        iterators equal to or that come after
        the first erased element are invalidated.
        Otherwise, no iterators are invalidated.

        @par Complexity
        Linear in `this->string().size()`.

        @return The number of headers erased.

        @param id The field name constant.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(field id) noexcept;

    /** Erase all matching fields.

        This removes all headers with a matching
        name, using a case-insensitive comparison.

        If any headers are erased, then all
        iterators equal to or that come after
        the first erased element are invalidated.
        Otherwise, no iterators are invalidated.

        @par Complexity
        Linear in `this->string().size()`.

        @return The number of fields erased

        @param name The header name.
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(
        core::string_view name) noexcept;

    //--------------------------------------------

    /** Set a header value.

        Uses the given value to overwrite the
        current one in the header field pointed to
        by the iterator. The value must be
        syntactically valid or else an error is
        returned.

        Any leading or trailing whitespace in the
        new value is ignored.

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param it The iterator to the header.

        @param value The value which must be semantically
        valid for the message.
    */
    void
    set(
        iterator it,
        core::string_view value)
    {
        system::error_code ec;
        set(it, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
    }

    /** Set a header value.

        Uses the given value to overwrite the
        current one in the header field pointed to
        by the iterator. The value must be
        syntactically valid or else an error is
        returned.

        Any leading or trailing whitespace in the
        new value is ignored.

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param it The iterator to the header.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set(
        iterator it,
        core::string_view value,
        system::error_code& ec);

    /** Set a header value.

        The container is modified to contain
        exactly one field with the specified id
        set to the given value, which must be
        syntactically valid or else an error is
        returned.

        Any leading or trailing whitespace in the
        new value is ignored.

        @par Postconditions
        @code
        this->count( id ) == 1 && this->at( id ) == value
        @endcode

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param id The field constant of the header
        to set.

        @param value The value which must be semantically
        valid for the message.
    */
    void
    set(
        field id,
        core::string_view value)
    {
        system::error_code ec;
        set(id, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
    }

    /** Set a header value.

        The container is modified to contain
        exactly one field with the specified id
        set to the given value, which must be
        syntactically valid or else an error is
        returned.

        Any leading or trailing whitespace in the
        new value is ignored.

        @par Postconditions
        @code
        this->count( id ) == 1 && this->at( id ) == value
        @endcode

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param id The field name constant.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set(
        field id,
        core::string_view value,
        system::error_code& ec);

    /** Set a header value.

        The container is modified to contain
        exactly one field with the specified name
        set to the given value, which must be
        syntactically valid or else an error is
        returned.

        Any leading or trailing whitespace in the
        new value is ignored.

        @par Postconditions
        @code
        this->count( name ) == 1 && this->at( name ) == value
        @endcode

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param name The field name.

        @param value The value which must be semantically
        valid for the message.
    */
    void
    set(
        core::string_view name,
        core::string_view value)
    {
        system::error_code ec;
        set(name, value, ec);
        if(ec.failed())
            detail::throw_system_error(ec);
    }

    /** Set a header value.

        The container is modified to contain
        exactly one field with the specified name
        set to the given value, which must be
        syntactically valid or else an error is
        returned.

        Any leading or trailing whitespace in the
        new value is ignored.

        @par Postconditions
        @code
        this->count( name ) == 1 && this->at( name ) == value
        @endcode

        @par Complexity

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param name The field name.

        @param value The value which must be semantically
        valid for the message.

        @param ec Set to the error if input is invalid.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set(
        core::string_view name,
        core::string_view value,
        system::error_code& ec);

private:
    BOOST_HTTP_PROTO_DECL
    void
    copy_impl(
        detail::header const&);

    BOOST_HTTP_PROTO_DECL
    void
    insert_impl(
        optional<field> id,
        core::string_view name,
        core::string_view value,
        std::size_t before,
        system::error_code& ec);

    void
    insert_unchecked(
        optional<field> id,
        core::string_view name,
        core::string_view value,
        std::size_t before,
        bool has_obs_fold);

    void
    raw_erase(
        std::size_t) noexcept;

    void
    raw_erase_n(field, std::size_t) noexcept;

    std::size_t
    erase_all(
        std::size_t i0,
        field id) noexcept;

    std::size_t
    erase_all(
        std::size_t i0,
        core::string_view name) noexcept;

    std::size_t
    offset(
        std::size_t i) const noexcept;

    std::size_t
    length(
        std::size_t i) const noexcept;
};

} // http_proto
} // boost

#endif
