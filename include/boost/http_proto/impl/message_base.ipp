//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_MESSAGE_BASE_IPP
#define BOOST_HTTP_PROTO_IMPL_MESSAGE_BASE_IPP

#include <boost/http_proto/message_base.hpp>

namespace boost {
namespace http_proto {

void
message_base::
set_payload_size(
    std::uint64_t n)
{
    h_.pay.kind = payload::sized;
    h_.pay.size = n;

    //if(! is_head_response())
    if(true)
    {
        // comes first for exception safety
        set_content_length(n);

        set_chunked(false);
    }
    else
    {
        // VFALCO ?
    }
}

void
message_base::
set_content_length(
    std::uint64_t n)
{
    set(field::content_length,
        detail::number_string(n));
}

char*
message_base::
set_prefix_impl(
    std::size_t n)
{
    if( n > h_.prefix ||
        h_.buf == nullptr)
    {
        // allocate or grow
        if( n > h_.prefix &&
            static_cast<std::size_t>(
                n - h_.prefix) >
            static_cast<std::size_t>(
                max_off_t - h_.size))
            detail::throw_length_error(
                "too large",
                BOOST_CURRENT_LOCATION);
        auto n0 = detail::buffer_needed(
            n + h_.size - h_.prefix,
            h_.count);
        auto buf = new char[n0];
        if(h_.buf != nullptr)
        {
            std::memcpy(
                buf + n,
                h_.buf + h_.prefix,
                h_.size - h_.prefix);
            detail::header::table ft(
                h_.buf + h_.cap);
            h_.copy_table(buf + n0);
            delete[] h_.buf;
        }
        else
        {
            std::memcpy(
                buf + n,
                h_.cbuf + h_.prefix,
                h_.size - h_.prefix);
        }
        h_.buf = buf;
        h_.cbuf = buf;
        h_.size = static_cast<
            off_t>(h_.size +
                n - h_.prefix);
        h_.prefix = static_cast<
            off_t>(n);
        h_.cap = n0;
        return h_.buf;
    }

    // shrink
    std::memmove(
        h_.buf + n,
        h_.buf + h_.prefix,
        h_.size - h_.prefix);
    h_.size = static_cast<
        off_t>(h_.size -
            h_.prefix + n);
    h_.prefix = static_cast<
        off_t>(n);
    return h_.buf;
}

void
message_base::
set_chunked_impl(bool value)
{
    if(value)
    {
        // set chunked
        if(! h_.te.is_chunked )
        {
            append(
                field::transfer_encoding,
                "chunked");
            return;
        }
    }
    else
    {
        // clear chunked

    }
}

} // http_proto
} // boost

#endif
