//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_BASE_HPP
#define BOOST_HTTP_PROTO_REQUEST_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/request_view.hpp>

namespace boost {
namespace http_proto {

/** Provides message metadata for HTTP requests
*/
class request_base
    : public message_base
{
    friend class request;
    template<std::size_t>
    friend class static_request;

    request_base() noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(detail::kind::request)
    {
    }

    request_base(std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::request,
            storage_size)
    {
    }

    request_base(
        std::size_t storage_size,
        std::size_t max_storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::request,
            storage_size,
            max_storage_size)
    {
    }

    explicit
    request_base(core::string_view s)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(detail::kind::request, s)
    {
    }

    explicit
    request_base(detail::header const& ph)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(ph)
    {
    }

    request_base(
        detail::header const& ph,
        char* storage,
        std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(ph, storage, storage_size)
    {
    }

public:
    request_base(
        char* storage,
        std::size_t storage_size) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::request, storage, storage_size)
    {
    }

    request_base(
        core::string_view s,
        char* storage,
        std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::request, storage, storage_size, s)
    {
    }

    request_base(
        request_view const& other,
        char* storage,
        std::size_t storage_size)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(*other.ph_, storage, storage_size)
    {
    }

    /** Return a read-only view to the request
    */
    operator request_view() const noexcept
    {
        return request_view(ph_);
    }

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return the method as an integral constant

        If the method returned is equal to
        @ref method::unknown, the method may
        be obtained as a string instead, by
        calling @ref method_text.
    */
    http_proto::method
    method() const noexcept
    {
        return ph_->req.method;
    }

    /** Return the method as a string
    */
    core::string_view
    method_text() const noexcept
    {
        return core::string_view(
            ph_->cbuf,
            ph_->req.method_len);
    }

    /** Return the request-target string
    */
    core::string_view
    target() const noexcept
    {
        return core::string_view(
            ph_->cbuf +
                ph_->req.method_len + 1,
            ph_->req.target_len);
    }

    /** Return the HTTP-version
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

    /** Set the method of the request to the enum
    */
    void
    set_method(
        http_proto::method m)
    {
        set_impl(
            m,
            to_string(m),
            target(),
            version());
    }

    /** Set the method of the request to the string
    */
    void
    set_method(
        core::string_view s)
    {
        set_impl(
            string_to_method(s),
            s,
            target(),
            version());
    }

    /** Set the target string of the request

        This function sets the request-target.
        The caller is responsible for ensuring
        that the string passed is syntactically
        valid.
    */
    void
    set_target(
        core::string_view s)
    {
        set_impl(
            ph_->req.method,
            method_text(),
            s,
            version());
    }

    /** Set the HTTP version of the request
    */
    void
    set_version(
        http_proto::version v)
    {
        set_impl(
            ph_->req.method,
            method_text(),
            target(),
            v);
    }

    /** Set the method, target, and version of the request

        This is more efficient than setting the
        properties individually.
    */
    void
    set_start_line(
        http_proto::method m,
        core::string_view t,
        http_proto::version v)
    {
        set_impl(m, to_string(m), t, v);
    }

    /** Set the method, target, and version of the request

        This is more efficient than setting the
        properties individually.
    */
    void
    set_start_line(
        core::string_view m,
        core::string_view t,
        http_proto::version v)
    {
        set_impl(string_to_method(m), m, t, v);
    }

    /** Set the Expect header
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_expect_100_continue(bool b);

private:
    BOOST_HTTP_PROTO_DECL
    void
    set_impl(
        http_proto::method m,
        core::string_view ms,
        core::string_view t,
        http_proto::version v);
};

} // http_proto
} // boost

#endif
