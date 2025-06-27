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

struct stream
{
    using alloc_func = void*(*)(void*, unsigned int, unsigned int);
    using free_func = void(*)(void*, void*);

    unsigned char* next_in;   // next input byte
    unsigned int   avail_in;  // number of bytes available at next_in
    unsigned long  total_in;  // total number of input bytes read so far

    unsigned char* next_out;  // next output byte will go here
    unsigned int   avail_out; // remaining free space at next_out
    unsigned long  total_out; // total number of bytes output so far

    char*          msg;       // last error message, NULL if no error
    void*          state;     // not visible by applications

    alloc_func     zalloc;    // used to allocate internal state
    free_func      zfree;     // used to deallocate internal state
    void*          opaque;    // private data object passed to zalloc and zfree

    int            data_type; // best guess about the data type: binary or text
                                    // for deflate, or the decoding state for inflate
    unsigned long  adler;     // Adler-32 or CRC-32 value of the uncompressed data
    unsigned long  reserved;  // reserved for future use
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

/// Flush methods
enum flush
{
    no_flush      = 0,
    partial_flush = 1,
    sync_flush    = 2,
    full_flush    = 3,
    finish        = 4,
    block         = 5,
    trees         = 6
};

/// Compression levels
enum compression_level
{
    default_compression = -1,
    no_compression      = 0,
    best_speed          = 1,
    best_compression    = 9
};

/// Compression strategy
enum compression_strategy
{
    default_strategy = 0,
    filtered         = 1,
    huffman_only     = 2,
    rle              = 3,
    fixed            = 4
};

/// Possible values of the data_type field for deflate
enum data_type
{
    binary  = 0,
    text    = 1,
    ascii   = 1,
    unknown = 2
};

/// Compression method
enum compression_method
{
    deflated = 8
};

} // zlib
} // http_proto
} // boost

#include <boost/http_proto/service/impl/zlib_service.hpp>

#endif
