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
#include <boost/http_proto/detail/except.hpp>
#include <stddef.h>

namespace boost {
namespace http_proto {

//------------------------------------------------

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

serializer::
~serializer()
{
}

serializer::
serializer(
    std::size_t buffer_size)
    : ws_(buffer_size)
    , h_copy_(detail::kind::request)
{
}

void
serializer::
reset() noexcept
{
    ws_.clear();
    src_ = nullptr;
    bs_[0] = {};
    bs_[1] = {};
}

bool
serializer::
is_complete() const noexcept
{
    return
        bs_[0].size() == 0 &&
        bs_[1].size() == 0;
}

void
serializer::
set_header(
    request_view const& req)
{
    set_header_impl(req.ph_);
}

void
serializer::
set_header(
    request const& req)
{
    set_header_impl(&req.h_);
}

void
serializer::
set_header(
    response_view const& res)
{
    set_header_impl(res.ph_);
}

void
serializer::
set_header(
    response const& res)
{
    set_header_impl(&res.h_);
}

//------------------------------------------------

auto
serializer::
prepare() ->
    result<const_buffers>
{
    auto p = bs_;
    std::size_t n = 0;
    if(p[0].size() > 0)
        ++n;
    else
        ++p;
    if(src_)
    {
        error_code ec;
        const_buffers cb;
        src_->prepare(cb, ec);
        if(ec.failed())
            return ec;
        auto it = cb.begin();
        for(std::size_t i = 0;
            i < cb.size(); ++i)
            p[n++] = *it++;
    }
    return const_buffers(p, n);
}

void
serializer::
consume(std::size_t n) noexcept
{
    if(bs_[0].size() > 0)
    {
        if(bs_[0].size() >= n)
        {
            bs_[0] += n;
            return;
        }
        n -= bs_[0].size();
        bs_[0] = {};
    }
    src_->consume(n);
}

//------------------------------------------------

void
serializer::
set_header_impl(
    detail::header const& h)
{
    h_copy_ = h;
    h_ = &h_copy_;
    bs_[0] = { h_->cbuf, h_->size };
}

void
serializer::
set_header_impl(
    detail::header const* ph)
{
    h_ = ph;
    bs_[0] = { h_->cbuf, h_->size };
}

//------------------------------------------------

void
set_body(
    serializer& sr,
    string_view s)
{
    set_body<string_view_source>(sr, s);
}

} // http_proto
} // boost

#endif
