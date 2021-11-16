//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_CODEC_IMPL_DEFLATE_SERVICE_IPP
#define BOOST_HTTP_PROTO_CODEC_IMPL_DEFLATE_SERVICE_IPP

#include <boost/http_proto/codec/deflate_service.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/codec/codecs.hpp>
#include <boost/http_proto/codec/decoder.hpp>
#include <boost/http_proto/codec/encoder.hpp>

#if 0
#include <boost/beast/zlib/deflate_stream.hpp>
#include <boost/beast/zlib/inflate_stream.hpp>
#endif

namespace boost {
namespace http_proto {

namespace detail {

class deflate_decoder_service_impl
    : public context::service
    , public decoder_type
{
public:
    explicit
    deflate_decoder_service_impl(
        context& ctx)
    {
        ctx.codecs().add_decoder(
            "deflate", *this);
    }

    class decoder_impl
        : public decoder
    {
#if 0
        beast::zlib::inflate_stream is_;
#endif

    public:
        void
        exchange(
            buffers& b,
            error_code& ec)
        {
            (void)b;
            (void)ec;
#if 0
            beast::zlib::z_params zs;
            zs.next_in = b.input;
            zs.avail_in = b.input_avail;
            zs.total_in = 0;
            zs.next_out = b.output;
            zs.avail_out = b.output_avail;
            zs.total_out = 0;

            is_.write(zs,
                beast::zlib::Flush::none,
                    ec);

            b.input = static_cast<
                char const*>(zs.next_in);
            b.input_avail = zs.avail_in;
            b.input_used = zs.total_in;
            b.output = static_cast<
                char*>(zs.next_out);
            b.output_avail = zs.avail_out;
            b.output_used = zs.total_out;
#endif
        }
    };

    std::unique_ptr<decoder>
    make_decoder() override
    {
        return std::unique_ptr<decoder>(
            new decoder_impl());
    }
};

//------------------------------------------------

class deflate_encoder_service_impl
    : public context::service
    , public encoder_type
{
public:
    explicit
    deflate_encoder_service_impl(
        context& ctx)
    {
        ctx.codecs().add_encoder(
            "deflate", *this);
    }

    class encoder_impl
        : public encoder
    {
#if 0
        beast::zlib::deflate_stream is_;
#endif

    public:
        void
        exchange(
            buffers& b,
            error_code& ec)
        {
            (void)b;
            (void)ec;
        }
    };

    std::unique_ptr<encoder>
    make_encoder() override
    {
        return std::unique_ptr<encoder>(
            new encoder_impl());
    }
};

} // detail

//------------------------------------------------

void
install_deflate_encoder(
    context& ctx)
{
    (void)ctx;
}

void
install_deflate_decoder(
    context& ctx)
{
    make_service<
        detail::deflate_decoder_service_impl>(ctx);
}

} // http_proto
} // boost

#endif
