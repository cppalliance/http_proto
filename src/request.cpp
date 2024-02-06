//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_view.hpp>
#include "detail/copied_strings.hpp"
#include "detail/number_string.hpp"
#include <utility>

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
            erase(it);
            return;
        }

        auto it = find(field::expect);
        BOOST_ASSERT(it != end());
        erase(it);
        return;
    }

    if(b)
    {
        if(! h_.md.expect.ec.failed())
        {
            // remove all but one
            raw_erase_n(
                field::expect,
                h_.md.expect.count - 1);
            return;
        }

        erase(field::expect);
        append(
            field::expect,
            "100-continue");
        return;
    }

    erase(field::expect);
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
    detail::copied_strings cs(
        this->buffer());
    ms = cs.maybe_copy(ms);
    t = cs.maybe_copy(t);

    auto const vs =
        to_string(v);
    auto const n =
        ms.size() + 1 +
        t.size() + 1 +
        vs.size() + 2;
    auto dest = set_prefix_impl(n);
    std::memcpy(
        dest,
        ms.data(),
        ms.size());
    dest += ms.size();
    *dest++ = ' ';
    std::memcpy(
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
