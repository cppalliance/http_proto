//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_GZIP_CODECS_IPP
#define BOOST_HTTP_PROTO_DETAIL_GZIP_CODECS_IPP

#include <boost/http_proto/detail/gzip_codec.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/core/ignore_unused.hpp>
#include <cstring>
#include "zlib.h"

namespace boost {
namespace http_proto {
namespace detail {

enum class gzip_error
{
    ok          =  0,
    stream_end  =  1,
    need_dict   =  2,
    errno_      = -1,
    stream_err  = -2,
    data_err    = -3,
    mem_err     = -4,
    buf_err     = -5,
    version_err = -6
};

//------------------------------------------------
} // detail
} // http_proto
namespace system {
template<>
struct is_error_code_enum<
    ::boost::http_proto::detail::gzip_error>
{
    static bool const value = true;
};
} // system
namespace http_proto {
namespace detail {
//------------------------------------------------

error_code
make_error_code(
    gzip_error ev) noexcept
{
    struct cat_t : error_category
    {
        cat_t() noexcept
            : error_category(
                0xe6c6d0215d1d6e22)
        {
        }
            
        const char*
        name() const noexcept override
        {
            return "boost.http.proto.gzip_error";
        }

        std::string
        message(int ev) const override
        {
            switch(static_cast<gzip_error>(ev))
            {
            case gzip_error::ok: return "Z_OK";
            case gzip_error::stream_end: return "Z_STREAM_END";
            case gzip_error::need_dict: return "Z_NEED_DICT";
            case gzip_error::errno_: return "Z_ERRNO";
            case gzip_error::stream_err: return "Z_STREAM_ERROR";
            case gzip_error::data_err: return "Z_DATA_ERROR";
            case gzip_error::mem_err: return "Z_MEM_ERROR";
            case gzip_error::buf_err: return "Z_BUF_ERROR";
            case gzip_error::version_err: return "Z_VERSION_ERROR";
            default:
                return "unknown";
            }
        }
    };
    static cat_t const cat{};
    return error_code{static_cast<
        std::underlying_type<
            error>::type>(ev), cat};
}

//------------------------------------------------

gzip_decoder::
~gzip_decoder()
{
    auto zs = reinterpret_cast<
        ::z_stream*>(zs_);
    if(zs)
    {
        ::inflateEnd(zs);
        delete zs;
    }
}

gzip_decoder::
gzip_decoder()
{
    ::z_stream* zs = new ::z_stream;
    zs->zalloc = nullptr;
    zs->zfree = nullptr;
    zs->opaque = nullptr;
    int const ec = ::inflateInit(zs);
    if(ec == Z_OK)
    {
        zs_ = zs;
        return;
    }
    delete zs;
    detail::throw_system_error(
        error_code(static_cast<
            gzip_error>(ec)));
}

gzip_decoder::
gzip_decoder(
    gzip_decoder&& other) noexcept
    : zs_(other.zs_)
{
    other.zs_ = nullptr;
}

auto
gzip_decoder::
exchange(
    void* output,
    std::size_t output_size,
    void const* input,
    std::size_t input_size) ->
        results
{
    auto zs = reinterpret_cast<
        ::z_stream*>(zs_);
    // VFALCO zlib seems to need const_cast
    zs->next_in =
        const_cast<Bytef z_const*>(
            reinterpret_cast<
                Bytef const*>(input));
    zs->avail_in = input_size;
    zs->next_out = reinterpret_cast<
        Bytef*>(output);
    zs->avail_in = output_size;
    results rv;
    rv.ec = static_cast<gzip_error>(
        ::inflate(zs, Z_NO_FLUSH));
    rv.input_used =
        input_size - zs->avail_in;
    rv.output_used =
        output_size - zs->avail_out;
    return rv;
}

//------------------------------------------------

gzip_encoder::
~gzip_encoder()
{
    auto zs = reinterpret_cast<
        ::z_stream*>(zs_);
    if(zs)
    {
        ::deflateEnd(zs);
        delete zs;
    }
}

gzip_encoder::
gzip_encoder()
{
    ::z_stream* zs = new ::z_stream;
    zs->zalloc = nullptr;
    zs->zfree = nullptr;
    zs->opaque = nullptr;
    int const ec = ::deflateInit(zs,
        Z_DEFAULT_COMPRESSION);
    if(ec == Z_OK)
    {
        zs_ = zs;
        return;
    }
    delete zs;
    detail::throw_system_error(
        error_code(static_cast<
            gzip_error>(ec)));
}

gzip_encoder::
gzip_encoder(
    gzip_encoder&& other) noexcept
    : zs_(other.zs_)
{
    other.zs_ = nullptr;
}

auto
gzip_encoder::
exchange(
    void* output,
    std::size_t output_size,
    void const* input,
    std::size_t input_size) ->
        results
{
    auto zs = reinterpret_cast<
        ::z_stream*>(zs_);
    // VFALCO zlib seems to need const_cast
    zs->next_in =
        const_cast<Bytef z_const*>(
            reinterpret_cast<
                Bytef const*>(input));
    zs->avail_in = input_size;
    zs->next_out = reinterpret_cast<
        Bytef*>(output);
    zs->avail_in = output_size;
    results rv;
    rv.ec = static_cast<gzip_error>(
        ::deflate(zs, Z_NO_FLUSH));
    rv.input_used =
        input_size - zs->avail_in;
    rv.output_used =
        output_size - zs->avail_out;
    return rv;
}

} // detail
} // http_proto
} // boost

#endif
