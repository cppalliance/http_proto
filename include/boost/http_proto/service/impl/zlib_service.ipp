//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_IMPL_ZLIB_SERVICE_IPP
#define BOOST_HTTP_PROTO_SERVICE_IMPL_ZLIB_SERVICE_IPP

#include <boost/http_proto/service/zlib_service.hpp>
#include <boost/system/result.hpp>
#include "zlib.h"

namespace boost {
namespace http_proto {
namespace zlib {
namespace detail {

/*
    DEFLATE Compressed Data Format Specification version 1.3
    https://www.rfc-editor.org/rfc/rfc1951
*/

//------------------------------------------------

enum class error
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
} // zlib
} // http_proto
namespace system {
template<>
struct is_error_code_enum<
    ::boost::http_proto::zlib::detail::error>
{
    static bool const value = true;
};
} // system
namespace http_proto {
namespace zlib {
namespace detail {
//------------------------------------------------

struct error_cat_type
    : system::error_category
{
    BOOST_SYSTEM_CONSTEXPR
    error_cat_type() noexcept
        : error_category(
            0xe6c6d0215d1d6e22)
    {
    }

    const char*
    name() const noexcept override
    {
        return "boost.http.proto.zlib";
    }

    std::string
    message( int ev ) const override
    {
        return message( ev, nullptr, 0 );
    }

    char const*
    message(
        int ev,
        char*,
        std::size_t) const noexcept override
    {
        switch(static_cast<error>(ev))
        {
        case error::ok: return "Z_OK";
        case error::stream_end: return "Z_STREAM_END";
        case error::need_dict: return "Z_NEED_DICT";
        case error::errno_: return "Z_ERRNO";
        case error::stream_err: return "Z_STREAM_ERROR";
        case error::data_err: return "Z_DATA_ERROR";
        case error::mem_err: return "Z_MEM_ERROR";
        case error::buf_err: return "Z_BUF_ERROR";
        case error::version_err: return "Z_VERSION_ERROR";
        default:
            return "unknown";
        }
    }
};

system::error_code
make_error_code(
    error ev) noexcept
{
    static BOOST_SYSTEM_CONSTEXPR
        error_cat_type cat{};
    return system::error_code{static_cast<
        std::underlying_type<
            error>::type>(ev), cat};
}

//------------------------------------------------

// probes memory usage for a config
class probe
{
public:
    explicit
    probe() noexcept
    {
        zs_.zalloc = &zalloc;
        zs_.zfree = &zfree;
        zs_.opaque = this;
    }

    system::result<std::size_t>
    deflate_init(
        int level)
    {
        n_ = 0;
        system::error_code ec;
        ec = static_cast<error>(
            deflateInit(&zs_, level));
        if(ec.failed())
            return ec;
        Bytef tmp[24]{};
        zs_.next_in = &tmp[0];
        zs_.avail_in = 1;
        zs_.next_out = &tmp[1];
        zs_.avail_out = 23;
        ec = static_cast<error>(
            deflate(&zs_,
                Z_FINISH));
        if( ec.failed() &&
            ec != error::stream_end)
            return ec;
        ec = static_cast<error>(
            deflateEnd(&zs_));
        if(ec.failed())
            return ec;
        return n_;
    }

    system::result<std::size_t>
    deflate_init2(
        int level,
        int method,
        int windowBits,
        int memLevel,
        int strategy)
    {
        n_ = 0;
        system::error_code ec;
        ec = static_cast<error>(
            deflateInit2(&zs_,
                level,
                method,
                windowBits,
                memLevel,
                strategy));
        if(ec.failed())
            return ec;
        Bytef tmp[2];
        zs_.next_in = &tmp[0];
        zs_.avail_in = 0;
        zs_.next_out = &tmp[1];
        zs_.avail_out = 0;
        ec = static_cast<error>(
            deflate(&zs_,
                Z_FULL_FLUSH));
        if(ec.failed())
            return ec;
        ec = static_cast<error>(
            deflateEnd(&zs_));
        if(ec.failed())
            return ec;
        return n_;
    }

private:
    static void* zalloc(void* opaque,
        uInt num, uInt size)
    {
        auto& self =
            *reinterpret_cast<
                probe*>(opaque);
        self.n_ += num * size;
        return new char[num * size];
    }

    static void zfree(
        void*, void* address)
    {
        delete[] reinterpret_cast<
            char*>(address);
    }

    z_stream_s zs_{};
    std::size_t n_ = 0;
};

//------------------------------------------------

struct
    deflate_decoder_service_impl
    : deflate_decoder_service
{
    using key_type =
        deflate_decoder_service;

    explicit
    deflate_decoder_service_impl(
        context& ctx,
        config const& cfg)
        : cfg_(cfg)
    {
        (void)ctx;
        probe p;
        auto n0 = p.deflate_init(
            Z_DEFAULT_COMPRESSION).value();
        (void)n0;
    }

private:
    config cfg_;

    config const&
    get_config() const noexcept override
    {
        return cfg_;
    }

    std::size_t
    space_needed() const noexcept override
    {
        return 0;
    }

    filter&
    make_filter(http_proto::detail::workspace& ws) const override
    {
        filter* p;
        (void)ws;
        p = nullptr;
        return *p;
    }
};

} // detail

void
deflate_decoder_service::
config::
install(context& ctx)
{
    ctx.make_service<
        detail::deflate_decoder_service_impl>(*this);
}

} // zlib
} // http_proto
} // boost

#endif
