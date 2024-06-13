//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_IMPL_ZLIB_SERVICE_IPP
#define BOOST_HTTP_PROTO_SERVICE_IMPL_ZLIB_SERVICE_IPP

#include <boost/http_proto/service/zlib_service.hpp>

#include <boost/http_proto/metadata.hpp>
#include <boost/http_proto/detail/workspace.hpp>

#include <boost/assert/source_location.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/config.hpp>
#include <boost/system/result.hpp>
#include <boost/throw_exception.hpp>

#include <zlib.h>

#include "../../src/zlib_service.hpp"

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

BOOST_NOINLINE BOOST_NORETURN
void
throw_zlib_error(
    int e,
    source_location const& loc = BOOST_CURRENT_LOCATION)
{
    throw_exception(
        system::system_error(static_cast<error>(e)), loc);
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


namespace {
void* zalloc_impl(
    void* opaque,
    unsigned items,
    unsigned size)
{
    try
    {
        auto n = items * size;
        auto* ws =
            reinterpret_cast<
                http_proto::detail::workspace*>(opaque);

        return ws->reserve_front(n);
    }
    catch(std::length_error const&) // represents OOM
    {
        return Z_NULL;
    }
}

void zfree_impl(void* /* opaque */, void* /* addr */)
{
    // we call ws_.clear() before the serializer is reused
    // so all the allocations are passively freed
}

} // namespace

class BOOST_HTTP_PROTO_ZLIB_DECL
    deflate_filter final : public filter
{
private:
    z_stream stream_;
    http_proto::detail::workspace& ws_;

    void init(bool use_gzip);

public:
    deflate_filter(
        http_proto::detail::workspace& ws,
        bool use_gzip = false);
    ~deflate_filter();

    deflate_filter(deflate_filter const&) = delete;
    deflate_filter& operator=(
        deflate_filter const&) = delete;

    filter::results
    on_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) override;
};

deflate_filter::
deflate_filter(
    http_proto::detail::workspace& ws,
    bool use_gzip)
    : ws_(ws)
{
    stream_.zalloc = &zalloc_impl;
    stream_.zfree = &zfree_impl;
    stream_.opaque = &ws_;
    init(use_gzip);
}

deflate_filter::
~deflate_filter()
{
    deflateEnd(&stream_);
}

void
deflate_filter::
init(bool use_gzip)
{
    int ret = -1;

    int window_bits = 15;
    if( use_gzip )
        window_bits += 16;

    int mem_level = 8;

    ret = deflateInit2(
        &stream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
        window_bits, mem_level, Z_DEFAULT_STRATEGY);

    if( ret != Z_OK )
        throw_zlib_error(ret);

    stream_.next_out = nullptr;
    stream_.avail_out = 0;

    stream_.next_in = nullptr;
    stream_.avail_in = 0;
}

filter::results
deflate_filter::
on_process(
    buffers::mutable_buffer out,
    buffers::const_buffer in,
    bool more)
{
    auto& zstream = stream_;

    auto flush = more ? Z_NO_FLUSH : Z_FINISH;
    int ret = -1;
    filter::results results;

    for(;;)
    {
        zstream.next_in =
            reinterpret_cast<unsigned char*>(
                const_cast<void*>(in.data()));
        zstream.avail_in = static_cast<unsigned>(
            in.size());

        zstream.next_out =
            reinterpret_cast<unsigned char*>(
                out.data());
        zstream.avail_out =
            static_cast<unsigned>(out.size());

        auto n1 = zstream.avail_in;
        auto n2 = zstream.avail_out;
        ret = deflate(&zstream, flush);

        in += (n1 - zstream.avail_in);
        out += (n2 - zstream.avail_out);

        results.in_bytes += (n1 - zstream.avail_in);
        results.out_bytes += (n2 - zstream.avail_out);

        auto is_empty = (in.size() == 0);

        if( ret != Z_OK &&
            ret != Z_BUF_ERROR &&
            ret != Z_STREAM_END )
            throw_zlib_error(ret);

        if( is_empty &&
            n2 == zstream.avail_out &&
            ret == Z_OK )
        {
            flush = Z_SYNC_FLUSH;
            continue;
        }

        if( ret == Z_STREAM_END )
            results.finished = true;

        if( ret == Z_BUF_ERROR )
            break;

        if( ret == Z_STREAM_END )
            break;

        if( ret == Z_OK &&
            out.size() == 0 )
            break;
    }
    return results;
}

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
    make_deflate_filter(
        http_proto::detail::workspace& ws) const override
    {
        return ws.emplace<deflate_filter>(ws, false);
    }

    filter&
    make_gzip_filter(
        http_proto::detail::workspace& ws) const override
    {
        return ws.emplace<deflate_filter>(ws, true);
    }
};

} // detail

void BOOST_HTTP_PROTO_ZLIB_DECL
install_deflate_encoder(context& ctx)
{
    detail::deflate_decoder_service::config cfg;
    ctx.make_service<
        detail::deflate_decoder_service_impl>(cfg);
}

} // zlib
} // http_proto
} // boost

#endif
