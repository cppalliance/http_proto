//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_BASE_HPP
#define BOOST_HTTP_PROTO_FIELDS_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view_base.hpp>

namespace boost {
namespace http_proto {

/** Mixin for modifiable HTTP fields

    @par Iterators

    Iterators obtained from @ref fields
    containers are not invalidated when
    the underlying container is modified.
*/
class BOOST_SYMBOL_VISIBLE
    fields_base
    : public fields_view_base
{
#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif
    friend class serializer;

    detail::header h_;

    BOOST_HTTP_PROTO_DECL
    explicit fields_base(detail::kind) noexcept;

    fields_base(detail::header const&);

public:
    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~fields_base();

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Returns the total number of bytes allocated by the container
    */
    std::size_t
    capacity_in_bytes() const noexcept
    {
        return h_.cap;
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Reserve additional storage
    */
    BOOST_HTTP_PROTO_DECL
    void
    reserve(std::size_t n);

    /** Remove excess capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    shrink_to_fit() noexcept;

    //--------------------------------------------

    /** Append the field with the given name and value.

        The name and value must contain only valid characters
        as specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional field with the same name is appended.

        @note HTTP field names are case-insensitive.

        @param value The corresponding value, which
        @li must be syntactically valid for the field,
        @li must be semantically valid for the message, and
        @li may not contain leading or trailing whitespace.
    */
    void
    append(
        field id,
        string_view value)
    {
        BOOST_ASSERT(
            id != field::unknown);
        insert_impl(
            id,
            to_string(id),
            value,
            h_.count);
    }

    /** Append the field with the given field enum and value.

        The value must contain only valid characters as
        specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same name is appended.

        @note HTTP field names are case-insensitive.

        @param value The corresponding value, which
        @li must be syntactically valid for the field,
        @li must be semantically valid for the message, and
        @li may not contain leading or trailing whitespace.
    */
    void
    append(
        string_view name,
        string_view value)
    {
        insert_impl(
            string_to_field(
                name),
            name,
            value,
            h_.count);
    }

    //--------------------------------------------

    /** Erase an element
    */
    void
    erase(iterator it) noexcept
    {
        erase_impl(it.i_, it->id);
    }

    /** Erase all matching fields

        @return The number of fields erased
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(field id) noexcept;

    /** Erase all matching fields

        @return The number of fields erased
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(string_view name) noexcept;

    //--------------------------------------------

    /** Insert a field

        @param value The corresponding value, which
        @li must be syntactically valid for the field,
        @li must be semantically valid for the message, and
        @li may not contain leading or trailing whitespace.
    */
    void
    insert(
        iterator before,
        field id,
        string_view value)
    {
        BOOST_ASSERT(
            id != field::unknown);
        insert_impl(
            id,
            to_string(id),
            value,
            before.i_);
    }

    /** Insert a field

        @param value The corresponding value, which
        @li must be syntactically valid for the field,
        @li must be semantically valid for the message, and
        @li may not contain leading or trailing whitespace.
    */
    void
    insert(
        iterator before,
        string_view name,
        string_view value)
    {
        insert_impl(
            string_to_field(
                name),
            name,
            value,
            before.i_);
    }

    //--------------------------------------------

    /** Set the value of a field
    */
    BOOST_HTTP_PROTO_DECL
    void
    set(
        iterator it,
        string_view value);

    /** Set the value of a field

        @param value The corresponding value, which
        @li must be syntactically valid for the field,
        @li must be semantically valid for the message, and
        @li may not contain leading or trailing whitespace.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set(
        field id,
        string_view value);

    /** Set the value of a field

        @param value The corresponding value, which
        @li must be syntactically valid for the field,
        @li must be semantically valid for the message, and
        @li may not contain leading or trailing whitespace.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set(
        string_view name,
        string_view value);

    //--------------------------------------------

    /** Return metadata about the Content-Length field
    */
    http_proto::content_length const&
    content_length() const noexcept
    {
        return h_.cl;
    }

    /** Set the Content-Length
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_content_length(
        std::uint64_t n);

#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif
    BOOST_HTTP_PROTO_DECL
    void
    clear_impl() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    copy_impl(detail::header const&);

    char*
    set_prefix_impl(
        std::size_t n);

private:
    std::size_t
    offset(
        detail::header::table ft,
        std::size_t i) const noexcept;

    std::size_t
    length(
        detail::header::table ft,
        std::size_t i) const noexcept;

    void
    raw_erase(
        std::size_t i) noexcept;

    std::size_t
    raw_erase_all(
        std::size_t i0) noexcept;

    void
    raw_insert(
        field id,
        string_view name,
        string_view value,
        std::size_t before);

    void
    raw_set(
        std::size_t i,
        string_view value);

    BOOST_HTTP_PROTO_DECL
    void
    erase_impl(
        std::size_t i,
        field id) noexcept;

    std::size_t
    erase_all_impl(
        std::size_t i0,
        field id) noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    insert_impl(
        field id,
        string_view name,
        string_view value,
        std::size_t before);
};

} // http_proto
} // boost

#endif
