//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BASIC_MESSAGE_HPP
#define BOOST_HTTP_PROTO_BASIC_MESSAGE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

/** Base class for request and response messages
*/
class basic_message
{
    // headers have a maximum size of 65536 chars
    using off_t = std::uint16_t;

    char* buf_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t size_ = 0;
    std::size_t n_start_ = 0;
    std::size_t n_field_ = 0;

public:
    BOOST_HTTP_PROTO_DECL
    basic_message();

    /** Return a string representing the entire serialized message
    */
    string_view
    data() const noexcept
    {
        // VFALCO Should this be null-terminated?
        if(buf_)
            return string_view(
                buf_, size_);
        return default_data();
    }

    /** Append the header with the given name and value.

        The name and value must contain only valid characters
        as specified in the HTTP protocol. The value should
        not include a trailing CRLF. If a matching header with
        the same name exists, it is not replaced. Instead, an
        additional header with the same field name is appended.

        @note HTTP field names are case-insensitive.
    */
    void
    emplace_back(
        field f,
        string_view value)
    {
        emplace_back(
            to_string(f), value);
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
    emplace_back(
        string_view name,
        string_view value)
    {
        emplace_back(
            string_to_field(name),
            name, value);
    }

#if 0
    /** Returns the value of a field if it exists, otherwise throws
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    at(field f) const noexcept;

    /** Returns the value of the first matching field if it exists, otherwise throws
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    at(string_view f) const noexcept;

    /** Returns the value of the first matching field if it exists, otherwise an empty string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value(field f) const noexcept;

    /** Returns the value of the first matching field if it exists, otherwise an empty string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value(string_view f) const noexcept;

    /** Returns the value of the first matching field if it exists, otherwise the given string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        field f,
        string_view) const noexcept;

    /** Returns the value of a field if it exists, otherwise the given string
    */
    BOOST_HTTP_PROTO_DECL
    string_view
    value_or(
        string_view f,
        string_view) const noexcept;

    struct matching_range;

    /** Returns a forward range of values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    matching_range
    matching(field f) const noexcept;

    /** Returns a forward range of values for all matching fields
    */
    BOOST_HTTP_PROTO_DECL
    matching_range
    matching(string_view f) const noexcept;

    /** Returns a range which contains all field-value pairs
    */
#endif

BOOST_HTTP_PROTO_PROTECTED:
    explicit
    basic_message(
        string_view start_line);

    BOOST_HTTP_PROTO_DECL
    void
    emplace_back(
        field f,
        string_view name,
        string_view value);

private:
    virtual string_view default_data() const noexcept = 0;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/basic_message.hpp>

#endif
