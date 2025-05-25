//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/service/deflate_service.hpp>
#include <zlib.h>
#include "src_zlib/service/stream_cast.hpp"

#include <boost/static_assert.hpp>

namespace boost {
namespace http_proto {
namespace zlib {

BOOST_STATIC_ASSERT(sizeof(stream_t) == sizeof(z_stream_s));
BOOST_STATIC_ASSERT(is_layout_identical<stream_t, z_stream_s>());

//------------------------------------------------

class deflate_service_impl
    : public deflate_service
    , public http_proto::service
{
public:
    using key_type = deflate_service;

    explicit
    deflate_service_impl(
        http_proto::context&) noexcept
    {
    }

    ~deflate_service_impl()
    {
    }

    char const*
    version() const noexcept override
    {
        return zlibVersion();
    }

    int
    init(
        stream_t& st,
        int level) const override
    {
        stream_cast sc(st);
        return deflateInit(sc.get(), level);
    }

    int
    init2(
        stream_t& st,
        int level,
        int method,
        int windowBits,
        int memLevel,
        int strategy) const override
    {
        stream_cast sc(st);
        return deflateInit2(sc.get(),
            level, method, windowBits,
            memLevel, strategy);
    }

    int
    set_dict(
        stream_t& st,
        unsigned char const* dict,
        unsigned len) const override
    {
        stream_cast sc(st);
        return deflateSetDictionary(sc.get(), dict, len);
    }

    int
    get_dict(
        stream_t& st,
        unsigned char* dest,
        unsigned* len) const override
    {
        stream_cast sc(st);
        return deflateGetDictionary(sc.get(), dest, len);
    }

    int
    dup(
        stream_t& dest,
        stream_t& src) const override
    {
        stream_cast sc0(dest);
        stream_cast sc1(src);
        return deflateCopy(sc0.get(), sc1.get());
    }

    int
    deflate(
        stream_t& st,
        int flush) const override
    {
        stream_cast sc(st);
        return ::deflate(sc.get(), flush);
    }

    int
    deflate_end(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return deflateEnd(sc.get());
    }

    int
    reset(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return deflateReset(sc.get());
    }

    int
    params(
        stream_t& st,
        int level,
        int strategy) const override
    {
        stream_cast sc(st);
        return deflateParams(sc.get(), level, strategy);
    }

    std::size_t
    bound(
        stream_t& st,
        unsigned long sourceLen) const override
    {
        stream_cast sc(st);
        return deflateBound(sc.get(), sourceLen);
    }

    int
    pending(
        stream_t& st,
        unsigned* pending,
        int* bits) const override
    {
        stream_cast sc(st);
        return deflatePending(sc.get(), pending, bits);
    }

    int
    prime(
        stream_t& st,
        int bits,
        int value) const override
    {
        stream_cast sc(st);
        return deflatePrime(sc.get(), bits, value);
    }

    int
    set_header(
        stream_t& st,
        void* header) const override
    {
        stream_cast sc(st);
        return deflateSetHeader(sc.get(),
            reinterpret_cast<gz_header*>(header));
    }
};

void
install_deflate_service(context& ctx)
{
    ctx.make_service<deflate_service_impl>();
}

} // zlib
} // http_proto
} // boost
