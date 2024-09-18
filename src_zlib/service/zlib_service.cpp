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

#include <boost/http_proto/metadata.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

#include <boost/assert/source_location.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/config.hpp>
#include <boost/system/result.hpp>
#include <boost/throw_exception.hpp>

#include <zlib.h>

namespace boost {
namespace http_proto {
namespace zlib {
namespace {

/*
    DEFLATE Compressed Data Format Specification version 1.3
    https://www.rfc-editor.org/rfc/rfc1951
*/

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

} // namespace
} // zlib
} // http_proto
namespace system {
template<>
struct is_error_code_enum<
    ::boost::http_proto::zlib::error>
{
    static bool const value = true;
};
} // system
namespace http_proto {
namespace zlib {
namespace {
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

struct service_impl
    : public service
{
    using key_type = service;

    static ::uInt
    clamp(std::size_t x) noexcept
    {
        if(x >= (std::numeric_limits<::uInt>::max)())
            return (std::numeric_limits<::uInt>::max)();
        return static_cast<::uInt>(x);
    }

    static void
    sync(
        z_stream* zs,
        buffers::mutable_buffer const& out,
        buffers::const_buffer const& in) noexcept
    {
        zs->next_in   = reinterpret_cast<
            unsigned char*>(const_cast<void*>(in.data()));
        zs->avail_in  = clamp(in.size());
        zs->next_out  = reinterpret_cast<
            unsigned char*>(out.data());
        zs->avail_out = clamp(out.size());
    }

    static stream::results
    make_results(
        z_stream const& zs,
        buffers::mutable_buffer const& out,
        buffers::const_buffer const& in,
        int ret) noexcept
    {
        return {
            clamp(out.size()) - zs.avail_out,
            clamp(in.size()) - zs.avail_in,
            ret < 0 ? static_cast<error>(ret) : system::error_code{},
            ret == Z_STREAM_END };
    }

    class deflator
        : public stream
    {
        z_stream zs_;

    public:
        deflator(
            http_proto::detail::workspace& ws,
            int level,
            int window_bits,
            int mem_level)
        {
            zs_.zalloc = &zalloc_impl;
            zs_.zfree  = &zfree_impl;
            zs_.opaque = &ws;

            auto ret = deflateInit2(&zs_, level, Z_DEFLATED,
                window_bits, mem_level, Z_DEFAULT_STRATEGY);
            if(ret != Z_OK)
                throw_zlib_error(ret);
        }

        virtual results
        write(
            buffers::mutable_buffer out,
            buffers::const_buffer in,
            flush flush) noexcept override
        {
            sync(&zs_, out, in);
            return make_results(zs_, out, in,
                deflate(&zs_, static_cast<int>(flush)));
        }
    };

    class inflator
        : public stream
    {
        z_stream zs_;

    public:
        inflator(
            http_proto::detail::workspace& ws,
            int window_bits)
        {
            zs_.zalloc = &zalloc_impl;
            zs_.zfree  = &zfree_impl;
            zs_.opaque = &ws;

            auto ret = inflateInit2(&zs_, window_bits);
            if(ret != Z_OK)
                throw_zlib_error(ret);
        }

        virtual results
        write(
            buffers::mutable_buffer out,
            buffers::const_buffer in,
            flush flush) noexcept override
        {
            sync(&zs_, out, in);
            return make_results(zs_, out, in,
                inflate(&zs_, static_cast<int>(flush)));
        }
    };

    explicit
    service_impl(context&) noexcept
    {
    }

    virtual std::size_t
    space_needed() const noexcept override
    {
        return 0; // TODO
    }

    virtual stream&
    make_deflator(
        http_proto::detail::workspace& ws,
        int level,
        int window_bits,
        int mem_level) const override
    {
        return ws.emplace<deflator>(
            ws, level, window_bits, mem_level);
    }

    virtual stream&
    make_inflator(
        http_proto::detail::workspace& ws,
        int window_bits) const override
    {
        return ws.emplace<inflator>(ws, window_bits);
    }
};

void BOOST_HTTP_PROTO_ZLIB_DECL
install_service(context& ctx)
{
    ctx.make_service<service_impl>();
}
} // zlib
} // http_proto
} // boost

#endif
