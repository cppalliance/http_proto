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

#ifndef BOOST_HTTP_PROTO_SERVICE_DEFLATE_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_DEFLATE_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

namespace boost {
namespace http_proto {
namespace zlib {

/** Provides the ZLib compression API
*/

struct BOOST_SYMBOL_VISIBLE
deflate_service
    : public http_proto::service
{
    virtual char const* version() const noexcept = 0;
    virtual int init(stream& st, int level) const = 0;
    virtual int init2(stream& st, int level, int method,
        int windowBits, int memLevel, int strategy) const = 0;
    virtual int set_dict(stream& st, unsigned char const* dict, unsigned len) const = 0;
    virtual int get_dict(stream& st, unsigned char* dest, unsigned* len) const = 0;
    virtual int dup(stream& dest, stream& src) const = 0;
    virtual int deflate(stream& st, int flush) const = 0;
    virtual int deflate_end(stream& st) const = 0;
    virtual int reset(stream& st) const = 0;
    virtual int params(stream& st, int level, int strategy) const = 0;
    virtual std::size_t bound(stream& st, unsigned long sourceLen) const = 0;
    virtual int pending(stream& st, unsigned* pending, int* bits) const = 0;
    virtual int prime(stream& st, int bits, int value) const = 0;
    virtual int set_header(stream& st, void* header) const = 0;
};

BOOST_HTTP_PROTO_ZLIB_DECL
void
install_deflate_service(context& ctx);

} // zlib
} // http_proto
} // boost

#endif
