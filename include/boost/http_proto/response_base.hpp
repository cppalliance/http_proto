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

#ifndef BOOST_HTTP_PROTO_RESPONSE_BASE_HPP
#define BOOST_HTTP_PROTO_RESPONSE_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/status.hpp>

namespace boost {
namespace http_proto {

/** Provides message metadata for HTTP responses
*/
class response_base
    : public message_base
{
    friend class response;
    template<std::size_t>
    friend class static_response;

    response_base() noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(detail::kind::response)
    {
    }

    response_base(std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::response,
            storage_size)
    {
    }

    response_base(
        std::size_t storage_size,
        std::size_t max_storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::response,
            storage_size,
            max_storage_size)
    {
    }

    explicit
    response_base(core::string_view s)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(detail::kind::response, s)
    {
    }

    explicit
    response_base(detail::header const& ph)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(ph)
    {
    }

    response_base(
        detail::header const& ph,
        char* storage,
        std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(ph, storage, storage_size)
    {
    }

public:
    response_base(
        char* storage,
        std::size_t storage_size) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::response, storage, storage_size)
    {
    }

    response_base(
        core::string_view s,
        char* storage,
        std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::response, storage, storage_size, s)
    {
    }

    response_base(
        response_view const& other,
        char* storage,
        std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(*other.ph_, storage, storage_size)
    {
    }

    /** Return a read-only view to the response
    */
    operator response_view() const noexcept
    {
        return response_view(ph_);
    }

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the reason string

        This field is obsolete in HTTP/1
        and should only be used for display
        purposes.
    */
    core::string_view
    reason() const noexcept
    {
        return core::string_view(
            ph_->cbuf + 13,
            ph_->prefix - 15);
    }

    /** Return the status code
    */
    http_proto::status
    status() const noexcept
    {
        return ph_->res.status;
    }

    /** Return the status code
    */
    unsigned short
    status_int() const noexcept
    {
        return ph_->res.status_int;
    }

    /** Return the HTTP version
    */
    http_proto::version
    version() const noexcept
    {
        return ph_->version;
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Set the version, status code of the response

        The reason phrase will be set to the
        standard text for the specified status
        code.

        @par sc The status code. This must not be
                @ref http_proto::status::unknown.

        @par v The HTTP-version.
    */
    void
    set_start_line(
        http_proto::status sc,
        http_proto::version v =
            http_proto::version::http_1_1)
    {
        set_impl(
            sc,
            static_cast<
                unsigned short>(sc),
            obsolete_reason(sc),
            v);
    }

    void
    set_start_line(
        unsigned short si,
        core::string_view reason,
        http_proto::version v)
    {
        set_impl(
            int_to_status(si),
            si,
            reason,
            v);
    }

private:
    BOOST_HTTP_PROTO_DECL
    void
    set_impl(
        http_proto::status sc,
        unsigned short si,
        core::string_view reason,
        http_proto::version v);
};

} // http_proto
} // boost

#endif
