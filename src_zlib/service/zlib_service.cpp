//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/service/zlib_service.hpp>

#include <boost/assert/source_location.hpp>
#include <boost/config.hpp>
#include <boost/system/result.hpp>
#include <boost/throw_exception.hpp>

#include <zlib.h>

namespace boost {
namespace http_proto {
namespace zlib {

namespace {

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

void* zalloc(
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

void zfree(void* /* opaque */, void* /* addr */)
{
    // we call ws_.clear() before the serializer is reused
    // so all the allocations are passively freed
}

static ::uInt
clamp(std::size_t x) noexcept
{
    if(x >= (std::numeric_limits<::uInt>::max)())
        return (std::numeric_limits<::uInt>::max)();
    return static_cast<::uInt>(x);
}

void
sync(z_stream* zs, params const& p) noexcept
{
    zs->next_in   = reinterpret_cast<::Bytef*>(
        const_cast<void*>(p.next_in));
    zs->avail_in  = clamp(p.avail_in);
    zs->next_out  = reinterpret_cast<::Bytef*>(p.next_out);
    zs->avail_out = clamp(p.avail_out);
}

void
sync(z_stream const& zs, params* p) noexcept
{
    p->next_in    = zs.next_in;
    p->avail_in  -= clamp(p->avail_in) - zs.avail_in;
    p->next_out   = zs.next_out;
    p->avail_out -= clamp(p->avail_out) - zs.avail_out;
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
        zs_.zalloc = &zalloc;
        zs_.zfree  = &zfree;
        zs_.opaque = &ws;

        auto ret = deflateInit2(&zs_, level, Z_DEFLATED,
            window_bits, mem_level, Z_DEFAULT_STRATEGY);
        if(ret != Z_OK)
            throw_zlib_error(ret);
    }

    system::error_code
    write(params& p, flush f) noexcept override
    {
        sync(&zs_, p);
        auto ret = deflate(&zs_, static_cast<int>(f));
        sync(zs_, &p);
        return static_cast<error>(ret);
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
        zs_.zalloc = &zalloc;
        zs_.zfree  = &zfree;
        zs_.opaque = &ws;

        auto ret = inflateInit2(&zs_, window_bits);
        if(ret != Z_OK)
            throw_zlib_error(ret);
    }

    system::error_code
    write(params& p, flush f) noexcept override
    {
        sync(&zs_, p);
        auto ret = inflate(&zs_, static_cast<int>(f));
        sync(zs_, &p);
        return static_cast<error>(ret);
    }
};

struct service_impl
    : public service
{
    using key_type = service;

    explicit
    service_impl(context&) noexcept
    {
    }

    std::size_t
    space_needed() const noexcept override
    {
        return 0; // TODO
    }

    stream&
    make_deflator(
        http_proto::detail::workspace& ws,
        int level,
        int window_bits,
        int mem_level) const override
    {
        return ws.emplace<deflator>(
            ws, level, window_bits, mem_level);
    }

    stream&
    make_inflator(
        http_proto::detail::workspace& ws,
        int window_bits) const override
    {
        return ws.emplace<inflator>(ws, window_bits);
    }
};

} // namespace

void
install_service(context& ctx)
{
    ctx.make_service<service_impl>();
}

} // zlib
} // http_proto
} // boost
