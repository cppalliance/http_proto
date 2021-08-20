//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_HEADERS_VIEW_HPP
#define BOOST_HTTP_PROTO_HEADERS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdlib>

namespace boost {
namespace http_proto {

/** A read-only, random access container of HTTP fields
*/
class headers_view
{
    char const* base_;
    std::size_t flen_;  // length of serialized fields
    std::size_t len_;   // size of storage
    std::size_t size_;  // number of fields

    headers_view(
        char const* base,
        std::size_t flen,
        std::size_t len,
        std::size_t size);

public:
    struct value_type
    {
        field id;
        string_view name;
        string_view value;
    };

    headers_view(
        headers_view const&) = default;
    headers_view& operator=(
        headers_view const&) = default;

    /** Constructor

        Default-constructed views have no elements.
    */
    BOOST_HTTP_PROTO_DECL
    headers_view();

    /** Return the serialized fields as a string
    */
    string_view
    str() const noexcept
    {
        return string_view(
            base_, flen_);
    }

    /** Returns the number of fields in the container
    */
    std::size_t
    size() const noexcept
    {
        return size_;
    }

    struct iterator;

    BOOST_HTTP_PROTO_DECL
    iterator begin() const noexcept;

    BOOST_HTTP_PROTO_DECL
    iterator end() const noexcept;

    /// Returns true if a field exists
    BOOST_HTTP_PROTO_DECL
    bool exists(field id);

    /// Returns true if a field exists
    BOOST_HTTP_PROTO_DECL
    bool exists(string_view name);

    /// Returns the number of matching fields
    BOOST_HTTP_PROTO_DECL
    std::size_t count(field id);

    /// Returns the number of matching fields
    BOOST_HTTP_PROTO_DECL
    std::size_t count(string_view name);

    /** Returns the value of the i-th field

        @par Preconditions
        @code
        i < size()
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    value_type const
    operator[](std::size_t i) const noexcept;  

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
    find(field id);

    /// Returns an iterator to the first matching field, otherwise returns end()
    BOOST_HTTP_PROTO_DECL
    iterator
    find(string_view name);

    struct subrange;

    /// Return a forward range containing values for all matching fields
    BOOST_HTTP_PROTO_DECL
    subrange
    matching(field id);

    /// Return a forward range containing values for all matching fields
    BOOST_HTTP_PROTO_DECL
    subrange
    matching(string_view name);

    /// Return a string containing a comma-separated list of values for all matching fields
    // VFALCO subrange::make_list() could subsume this
    //std::string list_all(field id);
    //std::string list_all(string_view name);
};

} // http_proto
} // boost

#include <boost/http_proto/impl/headers_view.hpp>

#endif
