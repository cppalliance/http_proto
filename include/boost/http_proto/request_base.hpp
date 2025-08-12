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

/** Mixin for modifing HTTP requests.

    @see
        @ref message_base,
        @ref request,
        @ref static_request.
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
        std::size_t cap)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(ph, storage, cap)
    {
    }

public:
    request_base(
        char* storage,
        std::size_t cap) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::request, storage, cap)
    {
    }

    request_base(
        core::string_view s,
        char* storage,
        std::size_t cap)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(
            detail::kind::request, storage, cap, s)
    {
    }

    request_base(
        request_view const& other,
        char* storage,
        std::size_t cap)
        : fields_view_base(
            &this->fields_base::h_)
        , message_base(*other.ph_, storage, cap)
    {
    }

    /** Conversion.

        @see
            @ref request_view.

        @return A view of the request.
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

    /** Return the method as a name constant.

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

    /** Return the method as a string.
    */
    core::string_view
    method_text() const noexcept
    {
        return core::string_view(
            ph_->cbuf,
            ph_->req.method_len);
    }

    /** Return the request-target string.
    */
    core::string_view
    target() const noexcept
    {
        return core::string_view(
            ph_->cbuf +
                ph_->req.method_len + 1,
            ph_->req.target_len);
    }

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    /** Set the method of the request to the enum.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param m The method to set.
    */
    void
    set_method(
        http_proto::method m)
    {
        set_start_line_impl(
            m,
            to_string(m),
            target(),
            version());
    }

    /** Set the method of the request to the string.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param s A string view representing the
        method to set.
    */
    void
    set_method(
        core::string_view s)
    {
        set_start_line_impl(
            string_to_method(s),
            s,
            target(),
            version());
    }

    /** Set the target string of the request.

        This function sets the request-target.
        The caller is responsible for ensuring
        that the string passed is syntactically
        valid.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param s A string view representing the
        target to set.
    */
    void
    set_target(
        core::string_view s)
    {
        set_start_line_impl(
            ph_->req.method,
            method_text(),
            s,
            version());
    }

    /** Set the HTTP version of the request.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param v The version to set.
    */
    void
    set_version(
        http_proto::version v)
    {
        set_start_line_impl(
            ph_->req.method,
            method_text(),
            target(),
            v);
    }

    /** Set the method, target, and version of the request.

        This is more efficient than setting the
        properties individually.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param m The method to set.

        @param t A string view representing the
        target to set.

        @param v The version to set.
    */
    void
    set_start_line(
        http_proto::method m,
        core::string_view t,
        http_proto::version v =
            http_proto::version::http_1_1)
    {
        set_start_line_impl(m, to_string(m), t, v);
    }

    /** Set the method, target, and version of the request.

        This is more efficient than setting the
        properties individually.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown on invalid input.
        Exception thrown if max capacity exceeded.

        @throw system_error
        Input is invalid.

        @throw std::length_error
        Max capacity would be exceeded.

        @param m A string view representing the
        method to set.

        @param t A string view representing the
        target to set.

        @param v The version to set.
    */
    void
    set_start_line(
        core::string_view m,
        core::string_view t,
        http_proto::version v =
            http_proto::version::http_1_1)
    {
        set_start_line_impl(string_to_method(m), m, t, v);
    }

    /** Set the `Expect: 100-continue` header.

        @par Exception Safety
        Strong guarantee.
        Calls to allocate may throw.
        Exception thrown if max capacity exceeded.

        @throw std::length_error
        Max capacity would be exceeded.

        @param b If `true` sets `Expect: 100-continue`
        header otherwise erase it.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_expect_100_continue(bool b);

private:
    BOOST_HTTP_PROTO_DECL
    void
    set_start_line_impl(
        http_proto::method m,
        core::string_view ms,
        core::string_view t,
        http_proto::version v);
};

} // http_proto
} // boost

#endif
