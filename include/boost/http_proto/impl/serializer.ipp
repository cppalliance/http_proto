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

//------------------------------------------------

bool
serializer::
is_complete() const noexcept
{
    return
        hs_.empty() &&
        bs_.empty();
}

void
serializer::
reset(header_info const& hi)
{
    hs_ = { hi.data, hi.size };
}

const_buffers
serializer::
prepare(error_code& ec)
{
    ec.clear();
    auto p = &v_[0];
    if(! hs_.empty())
    {
        *p = { hs_.data(), hs_.size() };
        ++p;
    }
    if(! bs_.empty())
    {
        *p = { bs_.data(), bs_.size() };
        ++p;
    }
    return const_buffers(v_, p - &v_[0]);
}

void
serializer::
consume(std::size_t n)
{
    if(n <= hs_.size())
    {
        hs_.remove_prefix(n);
        n = 0;
    }
    else
    {
        n -= hs_.size();
        hs_ = {};
    }

    if(n < bs_.size())
    {
        bs_.remove_prefix(n);
        n = 0;
    }
    else
    {
        n -= bs_.size();
        bs_ = {};
    }
}

#if 0
void
serializer::
clear() noexcept
{
    hs_ = {};
}

void
serializer::
reserve(std::size_t bytes)
{
    (void)bytes;
}

void
serializer::
reset_for_head(
    header_info const& hi)
{
    // TODO
    hs_ = { hi.data, hi.size };
}

void
serializer::
attach_body(string_view body)
{
    bs_ = body;
}

void
serializer::
attach_extensions(
    string_view extensions)
{
    // TODO
    (void)extensions;
}

void
serializer::
attach_end_of_body()
{
    // TODO
}

void
serializer::
attach_end_of_body(
    fields_view const& trailers)
{
    // TODO
    (void)trailers;
}

void
serializer::
flush()
{
    // TODO
    // VFALCO What do we flush here?
}

void
serializer::
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

const_buffers
serializer::
prepare(error_code& ec)
{
    // TODO
    (void)ec;
    return {};
}

const_buffers
serializer::
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
serializer::
peek_output() const noexcept
{
    // TODO
    return {};
}
#endif

} // http_proto
} // boost

#endif
