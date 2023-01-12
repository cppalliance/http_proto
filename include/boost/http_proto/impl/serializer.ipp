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
/*
    Rendering pipeline:

    buffers with source data ->
    encoding transformation ->
    chunking
*/
auto
serializer::
prepare() ->
    result<output_buffers>
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    if(src_)
    {
        // source body
        std::size_t n = 0;
        if(hbuf_.size() > 0)
        {
            pp_[n++] = hbuf_;
            if(is_expect_continue_)
                return output_buffers(pp_, n);
        }
        else if(is_expect_continue_)
        {
            is_expect_continue_ = false;
            BOOST_HTTP_PROTO_RETURN_EC(
                error::expect_100_continue);
        }
        auto rv = src_->read(buf_.prepare(
            buf_.capacity() - buf_.size()));
        // VFALCO partial success?
        if(rv.has_error())
            return rv.error();
        buf_.commit(rv->bytes);
        more_ = rv->more;
        for(const_buffer b : buf_.data())
            pp_[n++] = b;
        return output_buffers(pp_, n);
    }
    else if(bp_)
    {
        // buffers body
        std::size_t n = 0;
        if(hbuf_.size() > 0)
        {
            pp_[n++] = hbuf_;
            if(is_expect_continue_)
                return output_buffers(pp_, n);
        }
        else if(is_expect_continue_)
        {
            is_expect_continue_ = false;
            BOOST_HTTP_PROTO_RETURN_EC(
                error::expect_100_continue);
        }
        for(std::size_t i = 0; i < bn_; ++i)
            pp_[n++] = bp_[i];
        return output_buffers(pp_, n);
    }
    else
    {
        // empty body
        BOOST_ASSERT(hbuf_.size() > 0);
        pp_[0] = hbuf_;
        return output_buffers(pp_, 1);
    }
}

void
serializer::
consume(
    std::size_t n) noexcept
{
    if(src_)
    {
        // source body
        if(hbuf_.size() > 0)
        {
            if(n < hbuf_.size())
            {
                hbuf_ += n;
                return;
            }
            n -= hbuf_.size();
            hbuf_ = {};
        }
        buf_.consume(n);
        if( buf_.empty() &&
                ! more_)
            is_done_ = true;
        return;
    }
    else if(bp_)
    {
        // buffers body
        if(n < hbuf_.size())
        {
            hbuf_ += n;
            return;
        }
        n -= hbuf_.size();
        hbuf_ = {};
        while(n > 0)
        {
            if(n < bp_->size())
            {
                *bp_ += n;
                return;
            }
            n -= bp_->size();
            ++bp_;
            --bn_;
            if(bn_ == 0)
                break;
        }

        // Precondition violation
        if(n > 0)
            detail::throw_invalid_argument();

        is_done_ = true;
        return;
    }
    else
    {
        // empty body
        BOOST_ASSERT(hbuf_.size() > 0);
        if(n < hbuf_.size())
        {
            hbuf_ += n;
            return;
        }
        if(n == hbuf_.size())
        {
            is_done_ = true;
            return;
        }

        // Precondition violation
        detail::throw_invalid_argument();
    }
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
    // no body
    ws_.clear();
    src_ = nullptr;
    bp_ = nullptr;
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

    // headers
    hbuf_ = { m.ph_->cbuf, m.ph_->size };
    is_done_ = false;
    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;

    if(src_)
    {
        // source body
        pn_ = 3;
        pp_ = ws_.push_array(
            pn_, const_buffer{});
        buf_ = { ws_.data(), ws_.size() };
    }
    else if(bp_)
    {
        // buffers body
        pn_ = 1 + bn_;
        pp_ = ws_.push_array(
            pn_, const_buffer{});
    }
    else
    {
        // empty body
        pp_ = &hbuf_;
        pn_ = 1;

        // VFALCO What do we do with
        // Expect: 100-continue?
    }
}

} // http_proto
} // boost

#endif
