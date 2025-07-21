//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_HPP
#define BOOST_HTTP_PROTO_FIELDS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/core/detail/string_view.hpp>
#include <initializer_list>

namespace boost {
namespace http_proto {

/** A modifiable container of HTTP fields
*/
class fields final
    : public fields_base
{
public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor

        Default-constructed fields have no
        name-value pairs.
    */
    BOOST_HTTP_PROTO_DECL
    fields() noexcept;

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    fields(
        core::string_view s);

    /** Constructor

        Construct a fields container which allocates
        `storage_size` bytes for storing the header.
        Attempting to grow the container beyond
        this amount will result in an exception.
        The storage is also used internally to store
        instances of an implementation-defined type.
        The requested number of bytes will be aligned
        accordingly (currently the alignment requirement is 4).

        <br/>

        This constructor is useful when an upper-bound size
        of the fields is known ahead of time and we want
        to prevent reallocations.

        <br/>

        Passing an initial storage size of `0` does not
        throw and the maximum capacity is set to an
        implementation-defined limit observable via
        @ref max_capacity_in_bytes().

        @param storage_size The initial and final size of
        the storage.

        @code
        boost::http_proto::fields
        make_fields(std::string_view server)
        {
            std::size_t size = 4096;
            // flds.buffer() is now stable
            boost::http_proto::fields flds(size);
            BOOST_ASSERT(
                flds.max_capacity_in_bytes(), 4096);

            // uses spare capacity so that reallocations
            // are avoided
            flds.append(
                boost::http_proto::field::server, server);
            flds.append(
                boost::http_proto::field::connection, "close");
            return flds;
        }
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    fields(
        std::size_t storage_size);

    /** Constructor

        Construct a fields container which allocates
        `storage_size` bytes for storing the header, with an
        upper limit of `max_storage_size`. Attempting to
        grow the container beyond its maximum will result in
        an exception. The storage is also used internally to
        store instances of an implementation-defined type.
        Both values will be aligned accordingly (currently
        the alignment requirement is 4).

        <br/>

        This constructor is useful when there's a best-fit
        guess for an initial header size but we still wish
        to permit reallocating and growing the container to
        some upper limit.

        <br/>

        Passing an initial size of `0` does not throw.

        @param storage_size The initial size of the storage.

        @param max_storage_size The maximum size of the
        allocated storage. Any operation that attempts to
        grow the container beyond this value throws
        `std::length_error`.

        @throws std::length_error Thrown if `size > max_size`

        @code
        boost::http_proto::fields
        make_fields(std::string_view host)
        {
            std::size_t size = 4096;
            boost::http_proto::fields flds(size, 2 * size);
            BOOST_ASSERT(
                flds.max_capacity_in_bytes(), 2 * 4096);

            // uses spare capacity so that reallocations
            // are avoided
            flds.append(
                boost::http_proto::field::host, host);
            flds.append(
                boost::http_proto::field::connection, "close");
            return flds;
        }
        @endcode
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    fields(
        std::size_t storage_size,
        std::size_t max_storage_size);

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
    fields(fields_view const& other);

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    fields&
    operator=(fields&& f) noexcept;

    /** Assignment
    */
    fields&
    operator=(fields const& f) noexcept
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Assignment
    */
    fields&
    operator=(fields_view const& f)
    {
        copy_impl(*f.ph_);
        return *this;
    }

    /** Conversion
    */
    operator fields_view() const noexcept
    {
        return fields_view(ph_);
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Swap this with another instance
    */
    void
    swap(fields& other) noexcept
    {
        h_.swap(other.h_);
        std::swap(max_cap_, other.max_cap_);
    }

    /** Swap two instances
    */
    // hidden friend
    friend
    void
    swap(
        fields& t0,
        fields& t1) noexcept
    {
        t0.swap(t1);
    }
};

} // http_proto
} // boost

#endif
