//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_MESSAGE_VIEW_BASE_HPP
#define BOOST_HTTP_PROTO_MESSAGE_VIEW_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view_base.hpp>

namespace boost {
namespace http_proto {

/** Provides read-only access to common metadata
    in HTTP request and response messages.

    This type is useful for accessing common
    properties shared by request and response
    messages.

    @see
        @ref message_base,
        @ref response_view,
        @ref request_view,
        @ref metadata,
        @ref response_parser,
        @ref request_parser.
*/
class message_view_base
    : public virtual fields_view_base
{
    friend class request_view;
    friend class response_view;
    friend class message_base;

    message_view_base() noexcept
        // VFALCO This ctor-init has to be
        // here even though it isn't called,
        // so nullptr doesn't matter.
        : fields_view_base(nullptr)
    {
    }

    explicit
    message_view_base(
        detail::header const* ph) noexcept
        : fields_view_base(ph)
    {
    }

public:
    //--------------------------------------------
    //
    // Metadata
    //
    //--------------------------------------------

    /** Return the type of payload of this message.
    */
    auto
    payload() const noexcept ->
        http_proto::payload
    {
        return ph_->md.payload;
    }

    /** Return the payload size.

        When @ref payload returns @ref payload::size,
        this function returns the number of octets
        in the actual message payload.

        @return The number of octets in the
        actual message payload.
    */
    std::uint64_t
    payload_size() const noexcept
    {
        BOOST_ASSERT(
            payload() == payload::size);
        return ph_->md.payload_size;
    }

    /** Return true if semantics indicate
        connection persistence.
    */
    bool
    keep_alive() const noexcept
    {
        return ph_->keep_alive();
    }

    /** Return metadata about the message.
    */
    auto
    metadata() const noexcept ->
        http_proto::metadata const&
    {
        return ph_->md;
    }

    /** Return true if the message is using a chunked
        transfer encoding.
    */
    bool
    chunked() const noexcept
    {
        return ph_->md.transfer_encoding.is_chunked;
    }

    /** Return the HTTP-version.
    */
    http_proto::version
    version() const noexcept
    {
        return ph_->version;
    }
};

} // http_proto
} // boost

#endif
