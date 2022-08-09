//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_SOURCE_HPP
#define BOOST_HTTP_PROTO_FILE_SOURCE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/file.hpp>
#include <boost/http_proto/serializer.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    file_source
{
    file f_;
    //std::uint64_t pos_;
    std::uint64_t n_;

public:
    BOOST_HTTP_PROTO_DECL
    file_source(
        file_source&&) noexcept;

    BOOST_HTTP_PROTO_DECL
    ~file_source();

    BOOST_HTTP_PROTO_DECL
    explicit
    file_source(
        file&& f,
        std::uint64_t offset = 0,
        std::uint64_t size =
            std::uint64_t(-1)) noexcept;

    BOOST_HTTP_PROTO_DECL
    bool
    more() const noexcept;

    BOOST_HTTP_PROTO_DECL
    std::size_t
    write(
        void* dest,
        std::size_t size,
        error_code& ec);
};

} // http_proto
} // boost

#endif
