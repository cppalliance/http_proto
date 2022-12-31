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
    last-chunk     = 1*("0") [ chunk-ext ] CRLF
    trailer-part   = *( header-field CRLF )

    chunk-size     = 1*HEXDIG
    chunk-ext      = *( ";" chunk-ext-name [ "=" chunk-ext-val ] 
    chunk-data     = 1*OCTET ; a sequence of chunk-size octets

    chunk-ext-name = token
    chunk-ext-val  = token / quoted-string

    2^64-1 = 16 hex digits

    xxxxxxxxxxxxxxxx\r\n        18
    {data}\r\n                   2
    0\r\n
    {trailer}\r\n
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
{
}

//------------------------------------------------

auto
serializer::
prepare() ->
    result<output_buffers>
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    std::size_t n = 0;

    if(hbuf_.size() > 0)
    {
        // header
        cb_[n++] = hbuf_;
        if( (! src_ && n == cbn_) ||
            is_expect_continue_)
        {
            return output_buffers(cb_, n);
        }
    }
    else if(is_expect_continue_)
    {
        is_expect_continue_ = false;
        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if(src_)
    {
        // source body
        auto dest = buf_.prepare();
        auto rv = src_->read(
            dest.first.data(),
            dest.first.size());
        if(rv.has_error())
            return rv.error();
        buf_.commit(rv->bytes);
        if( rv->more &&
            dest.second.size() > 0)
        {
            rv = src_->read(
                dest.second.data(),
                dest.second.size());
            if(rv.has_error())
                return rv.error();
            buf_.commit(rv->bytes);
        }
        more_ = rv->more;
        auto src = buf_.data();
        cb_[n++] = src.first;
        if(src.second.size() > 0)
            cb_[n++] = src.first;
        return output_buffers(cb_, n);
    }

    // buffers body
    if(hbuf_.size() > 0)
        return output_buffers(cb_, 1 + cbn_);
    return output_buffers(cb_ + 1, cbn_);
}

void
serializer::
consume(
    std::size_t n) noexcept
{
    auto p = cb_;

    // header
    if(hbuf_.size() > 0)
    {
        if(n < hbuf_.size())
        {
            hbuf_ += n;
            return;
        }

        n -= hbuf_.size();
        hbuf_ = {};
        ++p;
    }

    if(src_)
    {
        // source body
        buf_.consume(n);

        if( buf_.empty() &&
            ! more_)
        {
            is_done_ = true;
        }
        return;
    }

    auto p0 = p;
    while(n > 0)
    {
        if(n < p->size())
        {
            std::size_t const i =
                cb_ + cbn_ - p;
            *p += n;
            std::memmove(
                p0,
                p,
                i * sizeof(*p));
            cbn_ -= i;
            return;
        }

        n -= p->size();
        ++n;
        ++p;
    }
    is_done_ = true;
}

auto
serializer::
data() noexcept ->
    input_buffers
{
    return {};
}

void
serializer::
commit(
    std::size_t bytes,
    bool end)
{
    (void)bytes;
    (void)end;
}

void
serializer::
reset(
    message_view_base const& m)
{
    ws_.clear();
    cbn_ = 1;
    cb_ = ws_.push_array(
        cbn_, const_buffer{});
    src_ = nullptr;
    reset_impl(m);
}

//------------------------------------------------

void
serializer::
reset_impl(
    message_view_base const& m)
{
    // throw if there are
    // any metadata errors?
    //
    // m.ph_->md.maybe_throw();

    hbuf_ = { m.ph_->cbuf, m.ph_->size };
    is_done_ = false;
    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;

    if(src_)
    {
        // source body
        buf_ = { ws_.data(), ws_.size() };
    }
    else
    {
    }
}

} // http_proto
} // boost

#endif
