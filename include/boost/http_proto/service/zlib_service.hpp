//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_ZLIB_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_ZLIB_SERVICE_HPP

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/service/service.hpp>

namespace boost {
namespace http_proto {
namespace zlib {

struct decoder_config
{
    unsigned max_window_bits = 15;
    unsigned mem_level = 8;
};

//------------------------------------------------

constexpr
inline
std::size_t
encoding_size_hint(decoder_config cfg = {}) noexcept
{
    // from: https://www.zlib.net/zlib_tech.html
    //
    // Memory Footprint
    //
    // zlib's memory footprint can also be specified fairly
    // precisely. It is larger for compression than for
    // decompression, and the exact requirements depend on
    // how the library was compiled.
    //
    // The memory requirements for compression depend on two
    // parameters, windowBits and memLevel:
    //     deflate memory usage (bytes) = (1 << (windowBits+2)) + (1 << (memLevel+9)) + 6 KB
    return
        (1 << (cfg.max_window_bits + 2)) +
        (1 << (cfg.mem_level + 9)) +
        (6 * 1024);
}

struct BOOST_HTTP_PROTO_ZLIB_DECL
    service
    : http_proto::service
{
    struct stream
    {
        enum class flush
        {
            none,
            partial,
            sync,
            full,
            finish,
            block,
            trees
        };

        struct params
        {
            void const* next_in;
            std::size_t avail_in;
            void* next_out;
            std::size_t avail_out;
        };

        virtual bool
        write(
            params&,
            flush,
            system::error_code& ec) noexcept = 0;
    };

    virtual
    std::size_t
    space_needed() const noexcept = 0;

    virtual stream&
    make_deflator(
        http_proto::detail::workspace& ws,
        int level,
        int window_bits,
        int mem_level) const = 0;

    virtual stream&
    make_inflator(
        http_proto::detail::workspace& ws,
        int window_bits) const = 0;
};

void BOOST_HTTP_PROTO_ZLIB_DECL
install_service(context& ctx);

} // zlib
} // http_proto
} // boost

#endif
