//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STATIC_FIELDS_HPP
#define BOOST_HTTP_PROTO_STATIC_FIELDS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** A modifiable static container of HTTP fields.

    This container owns a collection of HTTP fields,
    represented by an inline buffer with fixed capacity.
    The contents may be inspected and modified,
    and the implementation maintains a useful
    invariant: changes to the fields always
    leave it in a valid state.

    @par Example
    @code
    static_fields<1024> fs;

    fs.set(field::host, "example.com");
    fs.set(field::accept_encoding, "gzip, deflate, br");
    fs.set(field::cache_control, "no-cache");

    assert(fs.buffer() ==
        "Host: example.com\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n");
    @endcode

    @par Invariants
    @code
    this->capacity_in_bytes() == Capacity && this->max_capacity_in_bytes() == Capacity 
    @endcode

    @tparam Capacity The maximum capacity in bytes.

    @see
        @ref fields,
        @ref fields_view.
*/
template<std::size_t Capacity>
class static_fields final
    : public fields_base
{
    alignas(entry)
    char buf_[Capacity];

public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor.

        A default-constructed fields container
        contains no name-value pairs.

        @par Example
        @code
        static_fields<1024> fs;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    static_fields() noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            detail::kind::fields,
            buf_,
            Capacity)
    {
    }


    /** Constructor.

        Constructs a fields container from the
        string `s`, which must contain valid
        HTTP fields or else an exception is thrown.
        The new fields container retains ownership
        by making a copy of the passed string.

        @par Example
        @code
        static_fields<1024> fs(
            "Server: Boost.HttpProto\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n");
        @endcode

        @par Postconditions
        @code
        this->buffer.data() != s.data()
        @endcode

        @par Complexity
        Linear in `s.size()`.

        @par Exception Safety
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        The input does not contain a valid HTTP fields.

        @throw std::length_error
        Max capacity would be exceeded.

        @param s The string to parse.
    */
    explicit static_fields(
        core::string_view s)
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            detail::kind::fields,
            buf_,
            Capacity,
            s)
    {
    }

    /** Constructor.

        The newly constructed object contains
        a copy of `f`.

        @par Postconditions
        @code
        this->buffer() == f.buffer() && this->buffer.data() != f.buffer().data()
        @endcode

        @par Complexity
        Linear in `f.size()`.

        @param f The fields container to copy.
    */
    static_fields(
        static_fields const& f) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            *f.ph_,
            buf_,
            Capacity)
    {
    }

    /** Constructor.

        The newly constructed object contains
        a copy of `f`.

        @par Postconditions
        @code
        this->buffer() == f.buffer() && this->buffer.data() != f.buffer().data()
        @endcode

        @par Complexity
        Linear in `f.size()`.

        @par Exception Safety
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param f The fields container to copy.
    */
    /** Constructor
    */
    static_fields(
        fields_view const& f)
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(
            *f.ph_,
            buf_,
            Capacity)
    {
    }

    /** Assignment.

        The contents of `f` are copied and
        the previous contents of `this` are
        discarded.

        @par Postconditions
        @code
        this->buffer() == f.buffer() && this->buffer().data() != f.buffer().data()
        @endcode

        @par Complexity
        Linear in `f.size()`.

        @param f The fields container to copy.

        @return A reference to this object.
    */
    static_fields&
    operator=(static_fields const& f) noexcept
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Assignment.

        The contents of `f` are copied and
        the previous contents of `this` are
        discarded.

        @par Postconditions
        @code
        this->buffer() == f.buffer() && this->buffer().data() != f.buffer().data()
        @endcode

        @par Complexity
        Linear in `f.size()`.

        @par Exception Safety
        Strong guarantee.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param f The fields container to copy.

        @return A reference to this object.
    */
    static_fields&
    operator=(fields_view const& f)
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Conversion.

        @see
            @ref fields_view.

        @return A view of the fields.
    */
    operator fields_view() const noexcept
    {
        return fields_view(ph_);
    }
};

} // http_proto
} // boost

#endif
