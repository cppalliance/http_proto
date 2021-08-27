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
#include <boost/http_proto/basic_header.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <string>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
// forward declarations
enum class field : unsigned short;
class headers_view;
#endif

class headers : public basic_header
{
    // headers have a maximum size of 2^32-1 chars
    using off_t = std::uint32_t;

    char* buf_;
    std::size_t cap_;
    string_view empty_;
    std::size_t count_;
    std::size_t start_len_;
    std::size_t fields_len_;

    // 0=none
    // 1=request
    // 2=response
    int owner_; 

    friend class request;
    friend class response;

    static string_view const s_empty_[3];

    static constexpr std::size_t
        max_header_size_ = ((off_t)(-1));

    constexpr
    static
    std::size_t
    align_up(
        std::size_t n) noexcept;

    constexpr
    static
    std::size_t
    bytes_needed(
        std::size_t size,
        std::size_t count) noexcept;

    explicit
    headers(
        string_view empty) noexcept;

    explicit
    headers(
        int owner) noexcept;

public:
    struct value_type
    {
        field id;
        string_view name;
        string_view value;
    };

    class iterator;
    class subrange;

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    ~headers();

    /** Constructor

        Default-constructed headers have no
        initial fields.
    */
    BOOST_HTTP_PROTO_DECL
    headers() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    headers(headers&& other) noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    headers(headers const& other);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    headers& operator=(headers&&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    headers& operator=(headers const&);

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    inline
    iterator
    begin() const noexcept;

    inline
    iterator
    end() const noexcept;

    BOOST_HTTP_PROTO_DECL
    operator headers_view() const noexcept;

    /** Returns the number of fields in the container
    */
    std::size_t
    size() const noexcept
    {
        return count_;
    }

    /** Returns the total number of bytes allocated by the container
    */
    std::size_t
    capacity_in_bytes() const noexcept
    {
        return cap_;
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

    /// Returns true if a field exists
    inline
    bool
    exists(field id) const noexcept;

    /// Returns true if a field exists
    inline
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
    value_type const
    at(std::size_t i) const
    {
        if(i < count_)
            return (*this)[i];
        detail::throw_invalid_argument(
            "bad index", BOOST_CURRENT_LOCATION);
    }

    /// Returns the value of the first matching field if it exists, otherwise throws
    inline
    string_view
    at(field id) const;

    /// Returns the value of the first matching field if it exists, otherwise throws
    inline
    string_view
    at(string_view name) const;

    /// Returns the value of the first matching field, otherwise returns the given string
    inline
    string_view
    value_or(
        field id,
        string_view v) const noexcept;

    /// Returns the value of the first matching field, otherwise returns the given string
    inline
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

    inline
    iterator
    find_next(
        iterator after,
        field id) const noexcept;

    inline
    iterator
    find_next(
        iterator after,
        string_view name) const noexcept;

    BOOST_HTTP_PROTO_DECL
    std::size_t
    find_next(
        std::size_t after,
        field id) const noexcept;

    BOOST_HTTP_PROTO_DECL
    std::size_t
    find_next(
        std::size_t after,
        string_view name) const noexcept;

    /// Return a forward range containing values for all matching fields
    inline
    subrange
    matching(field id) const noexcept;

    /// Return a forward range containing values for all matching fields
    inline
    subrange
    matching(string_view name) const noexcept;

    /** Return the serialized fields as a string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    get_const_buffer() const noexcept override;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Clear the contents, but not the capacity
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

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

    /** Append the header with the given name and value.

        The name and value must contain only valid characters
        as specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same field name is appended.

        @note HTTP field names are case-insensitive.
    */
    void
    append(
        field f,
        string_view value)
    {
        insert(f, to_string(f),
            value, count_);
    }

    /** Append the header with the given field enum and value.

        The value must contain only valid characters as
        specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same name is appended.

        @note HTTP field names are case-insensitive.
    */
    void
    append(
        string_view name,
        string_view value)
    {
        insert(string_to_field(name),
            name, value, count_);
    }

    /** Swap this with another instance
    */
    BOOST_HTTP_PROTO_DECL
    void
    swap(headers& other) noexcept;

    /** Swap two instances
    */
    friend
    void
    swap(
        headers& v1,
        headers& v2) noexcept
    {
        v1.swap(v2);
    }

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
    BOOST_HTTP_PROTO_DECL
    string_view
    str_impl() const noexcept;

    struct alloc_t
    {
        char* buf;
        std::size_t capacity;
    };

    alloc_t
    alloc(
        std::size_t size,
        std::size_t count);

    BOOST_HTTP_PROTO_DECL
    char*
    set_start_line(
        std::size_t n);

    BOOST_HTTP_PROTO_DECL
    void
    insert(
        field id,
        string_view name,
        string_view value,
        std::size_t before);
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
