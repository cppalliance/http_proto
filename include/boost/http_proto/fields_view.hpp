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
#include <boost/http_proto/basic_header.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/const_string.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
enum class field : unsigned short;
#endif

/** A read-only, forward range of HTTP fields
*/
class BOOST_SYMBOL_VISIBLE
    fields_view_base
{
#ifdef BOOST_HTTP_PROTO_DOCS
private:
#else
//protected:
public:
#endif
    char const* cbuf_ = nullptr;
    std::size_t buf_len_ = 0;
    off_t start_len_ = 0;
    off_t end_pos_ = 0;
    off_t count_ = 0;

    struct ctor_params
    {
        char const* cbuf = nullptr;
        std::size_t buf_len = 0;
        std::size_t start_len = 0;
        std::size_t end_pos = 0;
        std::size_t count = 0;
    };

    static string_view default_buffer(char) noexcept;
    static bool is_default(char const*) noexcept;
    void write_table(void*) const noexcept;
    void swap(fields_view_base& other) noexcept;

    BOOST_HTTP_PROTO_DECL
    explicit fields_view_base(ctor_params const&) noexcept;

    BOOST_HTTP_PROTO_DECL
    explicit fields_view_base(char) noexcept;

protected:
    fields_view_base(
        fields_view_base const&) = default;
    fields_view_base&
        operator=(fields_view_base const&) = default;

public:
    //--------------------------------------------
    //
    // Types
    //
    //--------------------------------------------

    class iterator;
    class subrange;

    /** A field
    */
    struct value_type
    {
        field id;
        std::string name;
        std::string value;
    };

    /** A field
    */
    struct reference
    {
        field id;
        string_view name;
        string_view value;

    #ifndef BOOST_HTTP_PROTO_DOCS
        reference const*
        operator->() const noexcept
        {
            return this;
        }
    #endif
    };

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Return a string representing the serialized data
    */
    string_view
    buffer() const noexcept
    {
        return string_view(
            cbuf_, end_pos_);
    }

    //--------------------------------------------
    //
    // Iterators
    //
    //--------------------------------------------

    /** Return an iterator to the beginning of the range of fields
    */
    BOOST_HTTP_PROTO_DECL
    iterator
    begin() const noexcept;

    /** Return an iterator to the end of the range of fields
    */
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
        return count_;
    }

    /** Return true if a field exists
    */
    BOOST_HTTP_PROTO_DECL
    bool
    exists(field id) const noexcept;

    /** Return true if a field exists
    */
    BOOST_HTTP_PROTO_DECL
    bool
    exists(string_view name) const noexcept;

    /** Return the number of matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(field id) const noexcept;

    /** Return the number of matching fields
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    count(string_view name) const noexcept;

    /** Return the value of the first matching field if it exists, otherwise throw
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    at(field id) const;

    /** Return the value of the first matching field if it exists, otherwise throw
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    at(string_view name) const;

    /** Return the value of the first matching field, otherwise return the given string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        field id,
        string_view v) const noexcept;

    /** Return the value of the first matching field, otherwise returns the given string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        string_view name,
        string_view v) const noexcept;

    /// Returns an iterator to the first matching field, otherwise returns end()
    BOOST_HTTP_PROTO_DECL
    iterator
    find(field id) const noexcept;

    /** Returns an iterator to the first matching field, otherwise returns end()
    */
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

    /** Return a forward range containing values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(field id) const noexcept;

    /** Return a forward range containing values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    subrange
    find_all(string_view name) const noexcept;
};

//------------------------------------------------

/** A read-only, forward range of HTTP fields
*/
class BOOST_SYMBOL_VISIBLE
    fields_view
    : public fields_view_base
{
#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif
    explicit
    fields_view(
        ctor_params const& init) noexcept
        : fields_view_base(init)
    {
    }

public:
    /** Constructor

        Default constructed field views
        have a zero size.
    */
    fields_view() noexcept
        : fields_view_base(0)
    {
    }

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    fields_view(
        fields_view const&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields_view&
    operator=(
        fields_view const&) noexcept;

    //--------------------------------------------

    /** Swap this with another instance
    */
    BOOST_HTTP_PROTO_DECL
    void
    swap(fields_view& other) noexcept;

    /** Swap two instances
    */
    friend
    void
    swap(
        fields_view& v1,
        fields_view& v2) noexcept
    {
        v1.swap(v2);
    }
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
    fields_view::subrange const& r,
    Allocator const& a = {});

} // http_proto
} // boost

#include <boost/http_proto/impl/fields_view.hpp>

#endif
