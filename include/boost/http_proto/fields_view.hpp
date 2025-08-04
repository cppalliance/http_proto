//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_VIEW_HPP
#define BOOST_HTTP_PROTO_FIELDS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {

/** A view to a valid HTTP headers section.

    Objects of this type represent a view to
    an HTTP fields container. That is, it acts
    like a `core::string_view` in terms of
    ownership. The caller is responsible for
    ensuring that the lifetime of the underlying
    buffer extends until it is no
    longer referenced.

    @see
        @ref fields,
        @ref static_fields,
*/
class fields_view
    : public fields_view_base
{
    friend class fields;
    template<std::size_t>
    friend class static_fields;

    fields_view(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
        BOOST_ASSERT(ph_->kind ==
            detail::kind::fields);
    }

public:

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    /** Constructor.

        A default-constructed fields views refer to
        a valid HTTP headers section that contains
        no name-value pairs, which always remains
        valid.

        @par Example
        @code
        fields fs;
        @endcode

        @par Postconditions
        @code
        this->buffer() == "\r\n"
        @endcode

        @par Complexity
        Constant.
    */
    fields_view() noexcept
        : fields_view_base(
            detail::header::get_default(
                detail::kind::fields))
    {
    }

    /** Constructor.

        After construction, both fields views
        reference the same underlying buffer.
        Ownership is not transferred.

        @par Postconditions
        @code
        this->buffer().data() == other.buffer().data()
        @endcode

        @par Complexity
        Constant.

        @param other The other view.
    */
    fields_view(
        fields_view const& other) noexcept = default;

    /** Assignment.

        After assignment, both fields views
        reference the same underlying buffer.
        Ownership is not transferred.

        @par Postconditions
        @code
        this->buffer().data() == other.buffer().data()
        @endcode

        @par Complexity
        Constant.

        @return A reference to this object.

        @param other The other view.
    */
    fields_view&
    operator=(
        fields_view const& other) noexcept = default;

    /** Destructor

        Any reference, iterator, or other view
        which reference the same underlying
        buffer remain valid.
    */
    ~fields_view() = default;

    //--------------------------------------------

    /** Swap.

        Exchanges the view with that of `other`.
        All iterators and references remain valid.

        If `this == &other`, this function call has no effect.

        @par Example
        @code
        fields f1;
        f1.set(field::accept, "text/html");
        fields f2;
        f2.set(field::connection, "keep-alive");
        fields_view v1 = f1;
        fields_view v2 = f2;
        v1.swap(v2);
        assert(v1.buffer() == "Connection: keep-alive\r\n\r\n" );
        assert(v2.buffer() == "Accept: text/html\r\n\r\n" );
        @endcode

        @par Complexity
        Constant.

        @param other The object to swap with.
    */
    void
    swap(fields_view& other) noexcept
    {
        auto ph = ph_;
        ph_ = other.ph_;
        other.ph_ = ph;
    }

    /** Swap.

        Exchanges the view of `v0` with
        another `v1`. All iterators and
        references remain valid.

        If `&v0 == &v1`, this function call has no effect.

        @par Example
        @code
        fields f1;
        f1.set(field::accept, "text/html");
        fields f2;
        f2.set(field::connection, "keep-alive");
        fields_view v1 = f1;
        fields_view v2 = f2;
        std::swap(v1, v2);
        assert(v1.buffer() == "Connection: keep-alive\r\n\r\n" );
        assert(v2.buffer() == "Accept: text/html\r\n\r\n" );
        @endcode

        @par Effects
        @code
        v0.swap(v1);
        @endcode

        @par Complexity
        Constant.

        @param v0 The first object to swap.
        @param v1 The second object to swap.

        @see
            @ref fields_view::swap
    */
    friend
    void
    swap(
        fields_view& v0,
        fields_view& v1) noexcept
    {
        v0.swap(v1);
    }
};

} // http_proto
} // boost

#endif
