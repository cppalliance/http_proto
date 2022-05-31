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
    chunked-body   = *chunk
                    last-chunk
                    trailer-part
                    CRLF

    chunk          = chunk-size [ chunk-ext ] CRLF
                    chunk-data CRLF
    chunk-size     = 1*HEXDIG
    last-chunk     = 1*("0") [ chunk-ext ] CRLF

    chunk-data     = 1*OCTET ; a sequence of chunk-size octets


    2^64-1 = 20 digits

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
consume(std::size_t n) noexcept
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

//------------------------------------------------

} // http_proto
} // boost

#endif
