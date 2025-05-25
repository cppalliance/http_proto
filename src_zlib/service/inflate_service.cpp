//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/service/inflate_service.hpp>
#include <zlib.h>
#include "src_zlib/service/stream_cast.hpp"

namespace boost {
namespace http_proto {
namespace zlib {

//------------------------------------------------

class inflate_service_impl
    : public inflate_service
    , public http_proto::service
{
public:
    using key_type = inflate_service;

    explicit
    inflate_service_impl(
        http_proto::context&) noexcept
    {
    }

    ~inflate_service_impl()
    {
    }

    char const*
    version() const noexcept override
    {
        return zlibVersion();
    }

    int
    init(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return inflateInit(sc.get());
    }

    int
    init2(
        stream_t& st,
        int windowBits) const override
    {
        stream_cast sc(st);
        return inflateInit2(sc.get(), windowBits);
    }

    int
    inflate(
        stream_t& st,
        int flush) const override
    {
        stream_cast sc(st);
        return ::inflate(sc.get(), flush);
    }

    int
    inflate_end(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return inflateEnd(sc.get());
    }

    int
    set_dict(
        stream_t& st,
        unsigned char const* dict,
        unsigned len) const override
    {
        stream_cast sc(st);
        return inflateSetDictionary(sc.get(), dict, len);
    }

    int
    get_dict(
        stream_t& st,
        unsigned char* dest,
        unsigned* len) const override
    {
        stream_cast sc(st);
        return inflateGetDictionary(sc.get(), dest, len);
    }

    int
    sync(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return inflateSync(sc.get());
    }

    int
    dup(
        stream_t& dest,
        stream_t& src) const override
    {
        stream_cast sc0(dest);
        stream_cast sc1(src);
        return inflateCopy(sc0.get(), sc1.get());
    }

    int
    reset(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return inflateReset(sc.get());
    }

    int
    reset2(
        stream_t& st,
        int windowBits) const override
    {
        stream_cast sc(st);
        return inflateReset2(sc.get(), windowBits);
    }

    int
    prime(
        stream_t& st,
        int bits,
        int value) const override
    {
        stream_cast sc(st);
        return inflatePrime(sc.get(), bits, value);
    }

    long
    mark(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return inflateMark(sc.get());
    }

    int
    get_header(
        stream_t& st,
        void* header) const override
    {
        stream_cast sc(st);
        return inflateGetHeader(sc.get(),
            reinterpret_cast<gz_headerp>(header));
    }

    int
    back_init(
        stream_t& st,
        int windowBits,
        unsigned char* window) const override
    {
        stream_cast sc(st);
        return inflateBackInit(sc.get(), windowBits, window);
    }

    int
    back_end(
        stream_t& st) const override
    {
        stream_cast sc(st);
        return inflateBackEnd(sc.get());
    }

    unsigned long
    compile_flags() const override
    {
        return zlibCompileFlags();
    }
};

void
install_inflate_service(context& ctx)
{
    ctx.make_service<inflate_service_impl>();
}

} // zlib
} // http_proto
} // boost
