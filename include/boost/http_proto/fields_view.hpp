//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_VIEW_HPP
#define BOOST_HTTP_PROTO_FIELDS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/url/const_string.hpp>
#include <memory>

namespace boost {
namespace http_proto {

/** A read-only, random access container of HTTP fields
*/
class BOOST_SYMBOL_VISIBLE
    fields_view
{
    char const* p_ = nullptr;
    detail::const_fields_table t_;
    off_t n_ = 0;
    off_t n_field_ = 0;
    off_t n_start_ = 0;

    friend struct fields_view_test;

    fields_view(
        string_view s,
        std::size_t n_field,
        void const* ptable) noexcept;

public:
    class iterator;
    class subrange;

    struct value_type
    {
        field id;
        std::string name;
        std::string value;
    };

    struct reference
    {
        field id;
        string_view name;
        string_view value;

        reference const*
        operator->() const noexcept
        {
            return this;
        }
    };

    BOOST_HTTP_PROTO_DECL
    fields_view(
        fields_view const&) noexcept;

    BOOST_HTTP_PROTO_DECL
    fields_view& operator=(
        fields_view const&) noexcept;

    BOOST_HTTP_PROTO_DECL
    fields_view() noexcept;

    BOOST_HTTP_PROTO_DECL
    explicit
    fields_view(string_view s);

    BOOST_HTTP_PROTO_DECL
    iterator
    begin() const noexcept;

    BOOST_HTTP_PROTO_DECL
    iterator
    end() const noexcept;

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Returns the number of fields in the container
    */
    std::size_t
    size() const noexcept
    {
        return n_field_;
    }

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

    /** Search [from, end), from==end is valid
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from,
        field id) const noexcept;

    /** Search [from, end), from==end is valid
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    find(
        iterator from, 
        string_view name) const noexcept;

    /// Return a forward range containing values for all matching fields
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(field id) const noexcept;

    /// Return a forward range containing values for all matching fields
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(string_view name) const noexcept;
};

//------------------------------------------------

/** Return a fields subrange as a comma separated string

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.2
*/
template<
    class Allocator =
        std::allocator<char>>
urls::const_string
make_list(
    fields_view::subrange r,
    Allocator const& a = {});

} // http_proto
} // boost

#include <boost/http_proto/impl/fields_view.hpp>

#endif
