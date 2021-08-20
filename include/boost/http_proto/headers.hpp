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
    BOOST_HTTP_PROTO_DECL
    ~headers();

    BOOST_HTTP_PROTO_DECL
    headers() noexcept;

    BOOST_HTTP_PROTO_DECL
    char*
    resize_prefix(
        std::size_t n);

    string_view
    str() const noexcept
    {
        return string_view(buf_,
            prefix_bytes_ +
            fields_bytes_ + 2);
    }

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /// Returns the number of field/value pairs
    std::size_t size() const noexcept;

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

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    struct iterator;
    struct const_iterator;

    iterator begin() noexcept;
    iterator end() noexcept;

    const_iterator begin() noexcept;
    const_iterator end() noexcept;

    /// Returns the HTTP-version of this message
    http_proto::version version() const noexcept;

    string_view const operator[](std::size_t) const noexcept;
    
    string_view const operator[](field f) const noexcept;

    string_view const operator[](string_view s) const noexcept;

    /// Returns the value of a field if it exists, otherwise throws
    string_view at(field f) const noexcept;
    string_view at(std::size_t index) const noexcept;

    /// Returns the value of the first matching field if it exists, otherwise throws
    string_view at(string_view f) const noexcept;

    /// Returns the value of the first matching field if it exists, otherwise the given string
    string_view value_or(field f, string_view) const noexcept;

    /// Returns the value of a field if it exists, otherwise the given string
    string_view value_or(string_view f, string_view) const noexcept;

    struct subrange;

    /// Returns a forward range of values for all matching headers
    subrange matching(field f) const noexcept;

    /// Returns a forward range of values for all matching headers
    subrange matching(string_view name) const noexcept;

    iterator find(field f);
    iterator find(string_view f);

    /// Clears the contents but not the capacity
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
    void
    append(
        field id,
        string_view name,
        string_view value);
};

} // http_proto
} // boost

#include <boost/http_proto/impl/headers.hpp>

#endif
