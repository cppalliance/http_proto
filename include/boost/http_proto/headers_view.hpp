//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_HEADERS_VIEW_HPP
#define BOOST_HTTP_PROTO_HEADERS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/basic_header.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdlib>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
// forward declared
enum class field : unsigned short;
#endif

/** A read-only, random access container of HTTP fields
*/
class BOOST_SYMBOL_VISIBLE
    headers_view
    : public basic_header
{
    char const* buf_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t count_ = 0;
    std::size_t start_len_ = 0;
    std::size_t fields_len_ = 0;

    friend class headers;
    friend class request_view;
    friend class response_view;

    char const*
    base() const noexcept
    {
        return buf_;
    }

    headers_view(
        char const* buf,
        std::size_t cap,
        std::size_t count,
        std::size_t start_len,
        std::size_t fields_len) noexcept
        : buf_(buf)
        , cap_(cap)
        , count_(count)
        , start_len_(start_len)
        , fields_len_(fields_len)
    {
    }

public:
    struct value_type
    {
        field id;
        string_view name;
        string_view value;
    };

    class iterator;
    class subrange;

    headers_view(
        headers_view const&) = default;
    headers_view& operator=(
        headers_view const&) = default;

    /** Constructor

        Default-constructed headers have no fields.
    */
    headers_view() = default;

    inline
    iterator
    begin() const noexcept;

    inline
    iterator
    end() const noexcept;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the serialized fields as a string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    get_const_buffer() const noexcept override;

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

private:
    string_view
    str_impl() const noexcept;

    std::size_t
    find(
        std::size_t after,
        field id) const noexcept;

    std::size_t
    find(
        std::size_t after,
        string_view name) const noexcept;
};

//------------------------------------------------

class headers_view::subrange
{
    headers_view const* h_ = nullptr;
    std::size_t first_ = 0;

    friend class headers_view;

    inline
    subrange(
        headers_view const* h,
        std::size_t first) noexcept;

public:
    using value_type =
        headers_view::value_type;

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

#include <boost/http_proto/impl/headers_view.hpp>

#endif
