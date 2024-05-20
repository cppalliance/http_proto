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

#include <boost/buffers/circular_buffer.hpp>
#include <boost/system/result.hpp>

#include <list>

#include "zlib.h"

namespace boost {
namespace http_proto {
namespace zlib {
namespace detail {

static
constexpr
std::size_t
crlf_len_ = 2;

// last-chunk     = 1*("0") [ chunk-ext ] CRLF
static
constexpr
std::size_t
last_chunk_len_ =
    1 + // "0"
    crlf_len_ +
    crlf_len_; // chunked-body termination requires an extra CRLF

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


namespace {
void* zalloc_impl(
    void* /* opaque */,
    unsigned items,
    unsigned size)
{
    try {
        return ::operator new(items * size);
    } catch(std::bad_alloc const&) {
        return Z_NULL;
    }
}

void zfree_impl(void* /* opaque */, void* addr)
{
    ::operator delete(addr);
}
} // namespace

struct zlib_filter_impl
{
    z_stream stream_;
    buffers::circular_buffer buf_;
    content_coding_type coding_ = content_coding_type::none;
    bool is_done_ = false;

    zlib_filter_impl(
        http_proto::detail::workspace& ws)
    {
        (void)ws;
        stream_.zalloc = &zalloc_impl;
        stream_.zfree = &zfree_impl;
        stream_.opaque = nullptr;
    }

    ~zlib_filter_impl()
    {
        deflateEnd(&stream_);
    }
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
    std::list<zlib_filter> filters_;

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
    make_filter(http_proto::detail::workspace& ws) override
    {
        filters_.emplace_back(ws);
        auto p_filter = &filters_.back();
        return *p_filter;
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

zlib_filter::zlib_filter(
    http_proto::detail::workspace& ws)
{
    impl_ = new detail::zlib_filter_impl(ws);
}

zlib_filter::~zlib_filter()
{
    delete impl_;
}

void zlib_filter::init()
{
    auto& coding_ = impl_->coding_;
    auto& stream_ = impl_->stream_;

    int ret = -1;

    int window_bits = 15;
    if( coding_ == content_coding_type::gzip )
        window_bits += 16;

    int mem_level = 8;

    ret = deflateInit2(
        &stream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
        window_bits, mem_level, Z_DEFAULT_STRATEGY);

    if( ret != Z_OK )
        throw ret;

    stream_.next_out = nullptr;
    stream_.avail_out = 0;

    stream_.next_in = nullptr;
    stream_.avail_in = 0;
}

void
zlib_filter::
reset(enum content_coding_type coding)
{
    auto& coding_ = impl_->coding_;
    auto& stream_ = impl_->stream_;

    BOOST_ASSERT(coding != content_coding_type::none);
    if( coding_ == coding )
    {
        int ret = -1;
        ret = deflateReset(&stream_);
        if( ret != Z_OK )
            throw ret;
    }
    else
    {
        if( coding_ != content_coding_type::none )
            deflateEnd(&stream_);
        coding_ = coding;
        init();
    }
}

bool
zlib_filter::
is_done() const noexcept
{
    return impl_->is_done_;
}

filter::results
zlib_filter::
on_process(
    buffers::mutable_buffer out,
    buffers::const_buffer in,
    bool more)
{
    auto& zfilter = *impl_;
    auto& zstream = zfilter.stream_;

    BOOST_ASSERT(out.size() > 6);

    auto flush = more ? Z_NO_FLUSH : Z_FINISH;
    int ret = -1337;
    filter::results results;

    while( true )
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
            throw ret;

        if( is_empty &&
            n2 == zstream.avail_out &&
            ret == Z_OK )
        {
            flush = Z_SYNC_FLUSH;
            continue;
        }

        if( ret == Z_STREAM_END )
            zfilter.is_done_ = true;

        if( ret == Z_BUF_ERROR )
            break;

        if( ret == Z_STREAM_END )
            break;

        if( ret == Z_OK &&
            out.size() <
                detail::last_chunk_len_ +
                detail::crlf_len_ + 1 )
            break;
    }
    return results;
}

} // zlib
} // http_proto
} // boost

#endif
