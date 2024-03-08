//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_view.hpp>

#include <cstring>
#include <utility>

#include "detail/header.hpp"

namespace boost {
namespace http_proto {

request::
request() noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::request)
{
}

request::
request(
    core::string_view s)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::request, s)
{

}

request::
request(
    request&& other) noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(
        detail::kind::request)
{
    swap(other);
}

request::
request(
    request const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(*other.ph_)
{
}

request::
request(
    request_view const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , message_base(*other.ph_)
{
}

request&
request::
operator=(
    request&& other) noexcept
{
    request temp(
        std::move(other));
    temp.swap(*this);
    return *this;
}

//------------------------------------------------

void
request::
set_expect_100_continue(bool b)
{
    if(h_.md.expect.count == 0)
    {
        BOOST_ASSERT(
            ! h_.md.expect.ec.failed());
        BOOST_ASSERT(
            ! h_.md.expect.is_100_continue);
        if( b )
        {
            append(
                field::expect,
                "100-continue");
            return;
        }
        return;
    }

    if(h_.md.expect.count == 1)
    {
        if(b)
        {
            if(! h_.md.expect.ec.failed())
            {
                BOOST_ASSERT(
                    h_.md.expect.is_100_continue);
                return;
            }
            BOOST_ASSERT(
                ! h_.md.expect.is_100_continue);
            auto it = find(field::expect);
            BOOST_ASSERT(it != end());
            set(it, "100-continue");
            return;
        }

        auto it = find(field::expect);
        BOOST_ASSERT(it != end());
        erase(it);
        return;
    }

    BOOST_ASSERT(h_.md.expect.ec.failed());

    auto nc = (b ? 1 : 0);
    auto ne = h_.md.expect.count - nc;
    if( b )
        set(find(field::expect), "100-continue");

    raw_erase_n(field::expect, ne);
    h_.md.expect.count = nc;
    h_.md.expect.ec = {};
    h_.md.expect.is_100_continue = b;
}

//------------------------------------------------

void
request::
set_impl(
    http_proto::method m,
    core::string_view ms,
    core::string_view t,
    http_proto::version v)
{
    auto const vs =
        to_string(v);
    auto const n =
        // method SP
        ms.size() + 1 +
        // request-target SP
        t.size() + 1 +
        // HTTP-version CRLF
        vs.size() + 2;

    detail::prefix_op op(*this, n);
    auto dest = op.prefix_.data();
    std::memmove(
        dest,
        ms.data(),
        ms.size());
    dest += ms.size();
    *dest++ = ' ';
    std::memmove(
        dest,
        t.data(),
        t.size());
    dest += t.size();
    *dest++ = ' ';
    std::memcpy(
        dest,
        vs.data(),
        vs.size());
    dest += vs.size();
    *dest++ = '\r';
    *dest++ = '\n';

    h_.version = v;
    h_.req.method = m;
    h_.req.method_len =
        static_cast<offset_type>(ms.size());
    h_.req.target_len =
        static_cast<offset_type>(t.size());

    h_.on_start_line();
}

} // http_proto
} // boost
