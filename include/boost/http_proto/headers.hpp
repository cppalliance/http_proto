//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_HEADERS_HPP
#define BOOST_HTTP_PROTO_HEADERS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>
#include <string>

namespace boost {
namespace http_proto {

// forward declared
enum class field : unsigned short;

class headers
{
    using off_t = std::uint16_t;

    char* buf_ = nullptr;
    std::size_t count_ = 0;
    std::size_t capacity_ = 0;
    std::size_t fields_bytes_ = 0;
    std::size_t prefix_bytes_ = 0;

    static std::size_t align_up(
        std::size_t n) noexcept;

public:
    struct value_type
    {
        field id;
        string_view name;
        string_view value;
    };

    class iterator;
    class subrange;

    BOOST_HTTP_PROTO_DECL
    ~headers();

    BOOST_HTTP_PROTO_DECL
    headers() noexcept;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the serialized fields as a string
    */
    string_view
    str() const noexcept
    {
        return string_view(buf_,
            prefix_bytes_ +
            fields_bytes_ + 2);
    }

    /** Returns the number of fields in the container
    */
    std::size_t
    size() const noexcept
    {
        return count_;
    }

    /** Returns the value of the i-th field

        @par Preconditions
        @code
        i < size()
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    value_type const
    operator[](std::size_t i) const noexcept;  

    inline
    iterator
    begin() const noexcept;

    inline
    iterator
    end() const noexcept;

    /// Returns true if a field exists
    BOOST_HTTP_PROTO_DECL
    bool
    exists(field id) const noexcept;

    /// Returns true if a field exists
    BOOST_HTTP_PROTO_DECL
    bool
    exists(string_view name) const noexcept;

    /// Returns the number of matching fields
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(field id) const noexcept;

    /// Returns the number of matching fields
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(string_view name) const noexcept;

    /// Returns the value of the i-th field, or throws if i>=size()
    BOOST_HTTP_PROTO_DECL
    value_type const
    at(std::size_t i) const;

    /// Returns the value of the first matching field if it exists, otherwise throws
    BOOST_HTTP_PROTO_DECL
    string_view
    at(field id) const;

    /// Returns the value of the first matching field if it exists, otherwise throws
    BOOST_HTTP_PROTO_DECL
    string_view
    at(string_view name) const;

    /// Returns the value of the first matching field, otherwise returns the given string
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        field id,
        string_view v) const noexcept;

    /// Returns the value of the first matching field, otherwise returns the given string
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        string_view name,
        string_view v) const noexcept;

    /// Returns an iterator to the first matching field, otherwise returns end()
    BOOST_HTTP_PROTO_DECL
    iterator
    find(field id) const noexcept;

    /// Returns an iterator to the first matching field, otherwise returns end()
    BOOST_HTTP_PROTO_DECL
    iterator
    find(string_view name) const noexcept;

    /// Return a forward range containing values for all matching fields
    BOOST_HTTP_PROTO_DECL
    subrange
    matching(field id) const noexcept;

    /// Return a forward range containing values for all matching fields
    BOOST_HTTP_PROTO_DECL
    subrange
    matching(string_view name) const noexcept;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    char*
    resize_prefix(
        std::size_t n);

    /** Reserve additional storage
    */
    BOOST_HTTP_PROTO_DECL
    void
    reserve(std::size_t n);

    /** Remove excess capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    shrink_to_fit();

    /** Clear the contents, but not the capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Append the header with the given name and value.

        The name and value must contain only valid characters
        as specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same field name is appended.

        @note HTTP field names are case-insensitive.
    */
    BOOST_HTTP_PROTO_DECL
    void
    append(
        field f,
        string_view value);

    /** Append the header with the given field enum and value.

        The value must contain only valid characters as
        specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same name is appended.

        @note HTTP field names are case-insensitive.
    */
    BOOST_HTTP_PROTO_DECL
    void
    append(
        string_view name,
        string_view value);

#if 0
    void clear() noexcept;
    void append(field f, string_view value);
    void append(string_view name, string_view value);
    void set(std::size_t index, string_view value);
    void set(field f, string_view value);
    void set(string_view name, string_view value);
    const_iterator erase(const_iterator it);
    void erase_first(field f);
    void erase_first(string_view name);
    void erase_all(field f);
    void erase_all(string_view name);
#endif

private:
    std::size_t
    find(
        std::size_t after,
        field id) const noexcept;

    std::size_t
    find(
        std::size_t after,
        string_view name) const noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    append(
        field id,
        string_view name,
        string_view value);
};

//------------------------------------------------

class headers::subrange
{
    headers const* h_ = nullptr;
    std::size_t first_ = 0;

    friend class headers;

    inline
    subrange(
        headers const* h,
        std::size_t first) noexcept;

public:
    using value_type =
        headers::value_type;

    class iterator;

    subrange(
        subrange const&) = default;
    subrange& operator=(
        subrange const&) = default;

    /** Constructor

        Default-constructed subranges are empty.
    */
    subrange() = default;

    inline
    iterator
    begin() const noexcept;

    inline
    iterator
    end() const noexcept;

    /** Return all values as a comma separated string

        @see
            https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.2
    */
    BOOST_HTTP_PROTO_DECL
    std::string
    make_list() const;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/headers.hpp>

#endif
