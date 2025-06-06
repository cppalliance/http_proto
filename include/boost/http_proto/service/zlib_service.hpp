//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2024 Mohammad Nejati
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

struct stream_t
{
    using alloc_func = void*(*)(void*, unsigned int, unsigned int);
    using free_func = void(*)(void*, void*);

    unsigned char*      next_in;    // next input byte
    unsigned int        avail_in;   // number of bytes available at next_in
    unsigned long       total_in;   // total number of input bytes read so far

    unsigned char*      next_out;   // next output byte will go here
    unsigned int        avail_out;  // remaining free space at next_out
    unsigned long       total_out;  // total number of bytes output so far

    char const*         msg;        // last error message, NULL if no error
    void*               state;      // not visible by applications

    alloc_func          zalloc;     // used to allocate internal state
    free_func           zfree;      // used to deallocate internal state
    void*               opaque;     // private data object passed to zalloc and zfree

    int                 data_type;  // best guess about the data type: binary or text
                                    // for deflate, or the decoding state for inflate
    unsigned long       adler;      // Adler-32 or CRC-32 value of the uncompressed data
    unsigned long       reserved;   // reserved for future use
};

/** Error codes returned from compression/decompression functions.

    Negative values are errors, positive values are used
    for special but normal events.
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

/// Flush methods.
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

/** Input and output buffers.

    The application must update `next_in` and `avail_in` when `avail_in`
    has dropped to zero. It must update `next_out` and `avail_out` when
    `avail_out` has dropped to zero.
*/
struct params
{
    /// Next input byte
    void const* next_in;

    /// Number of bytes available at `next_in`
    std::size_t avail_in;

    /// Next output byte
    void* next_out;

    /// Number of bytes remaining free at `next_out`
    std::size_t avail_out;
};

/// Abstract interface for deflator/inflator streams.
struct stream
{
    /** Call the underling compression/decompression algorithm.

        @param p The input and output buffers.

        @param f The flush method.

        @return The result of operation that contains a value
        of @ref error.
    */
    virtual system::error_code
    write(params& p, flush f) noexcept = 0;
};

/** Provides in-memory compression and decompression functions
    using zlib underneath.
*/
struct BOOST_SYMBOL_VISIBLE
    service
    : http_proto::service
{
    /** The memory requirements for deflator.

        @param window_bits The window size.

        @param mem_level The memory level.

        @return The memory requirements in bytes.
    */
    virtual
    std::size_t
    deflator_space_needed(
        int window_bits,
        int mem_level) const noexcept = 0;

    /** The memory requirements for inflator.

        @param window_bits The window size.

        @return The memory requirements in bytes.
    */
    virtual
    std::size_t
    inflator_space_needed(
        int window_bits) const noexcept = 0;

    /** Create a deflator stream by calling zlib `deflateInit2()`.

        @param ws A reference to the workspace used for constructing the
        deflator stream object and for storage used by zlib.

        @param level The compression level.

        @param window_bits The window size.

        @param mem_level Specifies how much memory should be allocated
        for the internal compression state.

        @return A reference to the created deflator stream.

        @throws std::length_error If there is insufficient free space in 
        @ref `http_proto::detail::workspace`.
    */
    virtual stream&
    make_deflator(
        http_proto::detail::workspace& ws,
        int level,
        int window_bits,
        int mem_level) const = 0;

    /** Create an inflator stream by calling zlib `inflateInit2()`.

        @param ws A reference to the workspace used for constructing the
        inflator stream object and for storage used by zlib.

        @param window_bits The window size.

        @return A reference to the created inflator stream.

        @throws std::length_error If there is insufficient free space in
        @ref `http_proto::detail::workspace`.
    */
    virtual stream&
    make_inflator(
        http_proto::detail::workspace& ws,
        int window_bits) const = 0;
};

/** Installs a zlib service on the provided context.

    @param ctx A reference to the @ref context where the service
    will be installed.

    @throw std::invalid_argument If the zlib service already
    exist on the context.
*/
BOOST_HTTP_PROTO_ZLIB_DECL
void
install_service(context& ctx);

} // zlib
} // http_proto
} // boost

#include <boost/http_proto/service/impl/zlib_service.hpp>

#endif
