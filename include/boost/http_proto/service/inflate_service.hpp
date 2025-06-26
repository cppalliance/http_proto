//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_INFLATE_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_INFLATE_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

namespace boost {
namespace http_proto {
namespace zlib {

/** Provides the ZLib decompression API
*/
struct BOOST_SYMBOL_VISIBLE
inflate_service
    : public http_proto::service
{
    virtual char const* version() const noexcept = 0;
    virtual int init(stream& st) const = 0;
    virtual int init2(stream& st, int windowBits) const = 0;
    virtual int inflate(stream& st, int flush) const = 0;
    virtual int inflate_end(stream& st) const = 0;
    virtual int set_dict(stream& st, unsigned char const* dict, unsigned len) const = 0;
    virtual int get_dict(stream& st, unsigned char* dest, unsigned* len) const = 0;
    virtual int sync(stream& st) const = 0;
    virtual int dup(stream& dest, stream& src) const = 0;
    virtual int reset(stream& st) const = 0;
    virtual int reset2(stream& st, int windowBits) const = 0;
    virtual int prime(stream& st, int bits, int value) const = 0;
    virtual long mark(stream& st) const = 0;
    virtual int get_header(stream& st, void* header) const = 0;
    virtual int back_init(stream& st, int windowBits, unsigned char* window) const = 0;
    virtual int back_end(stream& st) const = 0;
    virtual unsigned long compile_flags() const = 0;
};

BOOST_HTTP_PROTO_ZLIB_DECL
void
install_inflate_service(context& ctx);

} // zlib
} // http_proto
} // boost

#endif
