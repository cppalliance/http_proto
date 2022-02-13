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
#include <boost/http_proto/fields_view.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
namespace detail {
struct fields_table;
} // detail
#endif

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
    char* buf_ = nullptr;
    char kind_ = 0;

    struct ctor_params
        : fields_view_base::ctor_params
    {
        char* buf = nullptr;
        char kind = 0;
    };

    explicit fields_base(ctor_params const& init) noexcept;
    explicit fields_base(char kind) noexcept;
    fields_base(fields_view_base const& fv, char kind);

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
        return buf_len_;
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

    /** Append the field with the given name and value.

        The name and value must contain only valid characters
        as specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional field with the same name is appended.

        @note HTTP field names are case-insensitive.
    */
    BOOST_HTTP_PROTO_DECL
    void
    emplace_back(
        field id,
        string_view value);

    /** Append the field with the given field enum and value.

        The value must contain only valid characters as
        specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same name is appended.

        @note HTTP field names are case-insensitive.
    */
    BOOST_HTTP_PROTO_DECL
    void
    emplace_back(
        string_view name,
        string_view value);

    /** Insert a field
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    emplace(
        iterator before,
        field id,
        string_view value);

    /** Insert a field
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    emplace(
        iterator before,
        string_view name,
        string_view value);

    /** Erase an element
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    erase(iterator it) noexcept;

    /** Erase the first matching field

        @return The number of fields erased
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(field id) noexcept;

    /** Erase the first matching field

        @return The number of fields erased
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase(string_view name) noexcept;

    /** Erase all matching fields

        @return The number of fields erased
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase_all(field id) noexcept;

    /** Erase all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    erase_all(string_view name) noexcept;

#if 0
    void set(std::size_t index, string_view value);
    void set(field f, string_view value);
    void set(string_view name, string_view value);
#endif

private:
    std::size_t
    offset(
        detail::fields_table const& ft,
        std::size_t i) const noexcept;

    void
    insert_impl(
        field id,
        string_view name,
        string_view value,
        std::size_t before);

#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif
    BOOST_HTTP_PROTO_DECL
    void
    swap(
        fields_base& other) noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    char*
    set_start_line_impl(
        std::size_t n);
};

//------------------------------------------------

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

#if 0
    void set(std::size_t index, string_view value);
    void set(field f, string_view value);
    void set(string_view name, string_view value);
#endif
};

} // http_proto
} // boost

#include <boost/http_proto/impl/fields.hpp>

#endif
