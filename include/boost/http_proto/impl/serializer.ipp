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

serializer::
serializer(
    std::size_t buffer_size)
    : h_copy_(detail::kind::request)
{
    auto const Align =
        alignof(::max_align_t);
    cap_ = buffer_size;
    if( cap_ & (Align - 1))
        cap_ = (cap_ | (Align - 1)) + 1;
    if(cap_ < buffer_size)
        detail::throw_length_error(
            "buffer size",
            BOOST_CURRENT_LOCATION);
    buf_ = new char[cap_];
}

//------------------------------------------------

serializer::
~serializer()
{
    delete[] buf_;
}

void
serializer::
reset() noexcept
{
    v_[0] = {};
    v_[1] = {};
}

bool
serializer::
is_complete() const noexcept
{
    return
        v_[0].size() == 0 &&
        v_[1].size() == 0;
}

void
serializer::
set_header(
    request_view const& req)
{
    set_header_impl(req.h_);
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
    set_header_impl(res.h_);
}

void
serializer::
set_header(
    response const& res)
{
    set_header_impl(&res.h_);
}

//------------------------------------------------

const_buffers
serializer::
prepare(error_code& ec)
{
    ec.clear();
    if(v_[0].size() > 0)
    {
        if(v_[1].size() > 0)
            return const_buffers(v_, 2);
        return const_buffers(v_, 1);
    }
    return const_buffers(v_ + 1, 1);
}

void
serializer::
consume(std::size_t n) noexcept
{
    if(n <= v_[0].size())
    {
        v_[0] += n;
        return;
    }
    else
    {
        n -= v_[0].size();
        v_[0] = {};
    }
    v_[1] += n;
}

//------------------------------------------------

class string_view_source
    : public source
{
    asio::const_buffer b_;

public:
    explicit
    string_view_source(
        string_view s) noexcept
        : b_(s.data(), s.size())
    {
    }

    const_buffers
    prepare(error_code& ec) override
    {
        ec = {};
        return const_buffers(&b_, 1);
    }

    void
    consume(std::size_t n) noexcept override
    {
        b_ += n;
    }
};

void
serializer::
set_body(string_view s)
{
    v_[1] = { s.data(), s.size() };
}

//------------------------------------------------

void
serializer::
set_header_impl(
    detail::header const& h)
{
    h_copy_ = h;
    h_ = &h_copy_;
    v_[0] = { h_->cbuf, h_->size };
}

void
serializer::
set_header_impl(
    detail::header const* ph)
{
    h_ = ph;
    v_[0] = { h_->cbuf, h_->size };
}

} // http_proto
} // boost

#endif
