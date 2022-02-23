//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_IPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_IPP

#include <boost/http_proto/serializer.hpp>

namespace boost {
namespace http_proto {

/*

38 chars:
Content-Length: 18446744073709551615rn
12345678901234567890123456789012345678
         1         2         3       3

Transfer-Encoding: 
*/
serializer::
~serializer()
{
    if(buf_)
        delete[] buf_;
}

serializer::
serializer(
    context& ctx) noexcept
    : ctx_(ctx)
{
    (void)ctx_;
    (void)cap_;
    (void)size_;
}

//------------------------------------------------

void
serializer::
set_header(
    header_info const& hi) noexcept
{
    hi_ = hi;
    if(! hi_.meta)
    {
        // TODO
        // generate metadata
    }
}

void
serializer::
set_body(
    void const*,
    std::size_t) noexcept
{
}

void
serializer::
reset() noexcept
{
}

void
serializer::
clear() noexcept
{
}

const_buffer_pair
serializer::
prepare(
    error_code& ec)
{
    ec = {};
    const_buffer_pair p;
    p.data[0] = hs_.data();
    p.size[0] = hs_.size();
    p.data[1] = bs_.data();
    p.size[1] = bs_.size();
    return p;
}

void
serializer::
consume(
    std::size_t) noexcept
{
}

//------------------------------------------------
//
//
//
//------------------------------------------------

void
serializer_::
clear() noexcept
{
    hs_ = {};
}

void
serializer_::
reserve(std::size_t bytes)
{
    (void)bytes;
}

void
serializer_::
reset(header_info const& hi)
{
    hs_ = { hi.data, hi.size };
}

void
serializer_::
reset_for_head(
    header_info const& hi)
{
    // TODO
    hs_ = { hi.data, hi.size };
}

bool
serializer_::
is_complete() const noexcept
{
    return
        hs_.empty() &&
        bs_.empty();
}

void
serializer_::
attach_body(string_view body)
{
    bs_ = body;
}

void
serializer_::
attach_extensions(
    string_view extensions)
{
    // TODO
    (void)extensions;
}

void
serializer_::
attach_end_of_body()
{
    // TODO
}

void
serializer_::
attach_end_of_body(
    fields_view const& trailers)
{
    // TODO
    (void)trailers;
}

void
serializer_::
flush()
{
    // TODO
    // VFALCO What do we flush here?
}

void
serializer_::
consume(std::size_t bytes)
{
    if(! hs_.empty())
    {
        if(bytes < hs_.size())
        {
            hs_.remove_prefix(bytes);
            return;
        }
        bytes -= hs_.size();
        hs_ = {};
    }

    BOOST_ASSERT(bytes <= bs_.size());
    if(bytes < bs_.size())
    {
        bs_.remove_prefix(bytes);
        return;
    }
    bs_ = {};

    // is_complete_ = true; ?
}

buffers
serializer_::
prepare(error_code& ec)
{
    // TODO
    (void)ec;
    return {};
}

buffers
serializer_::
prepare(
    std::size_t bytes,
    error_code& ec)
{
    // TODO
    (void)bytes;
    (void)ec;
    return {};
}

string_view
serializer_::
peek_output() const noexcept
{
    // TODO
    return {};
}

} // http_proto
} // boost

#endif
