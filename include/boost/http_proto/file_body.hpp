//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_BODY_HPP
#define BOOST_HTTP_PROTO_FILE_BODY_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/file.hpp>
#include <boost/buffers/source.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    file_body
    : public buffers::source
{
    file f_;
    std::uint64_t n_;

public:
    file_body() = delete;
    file_body(
        file_body const&) = delete;

    BOOST_HTTP_PROTO_DECL
    file_body(
        file_body&&) noexcept;

    BOOST_HTTP_PROTO_DECL
    ~file_body();

    BOOST_HTTP_PROTO_DECL
    explicit
    file_body(
        file&& f,
        std::uint64_t size =
            std::uint64_t(-1)) noexcept;

    BOOST_HTTP_PROTO_DECL
    results
    read_one(
        void* dest,
        std::size_t size) override;
};

} // http_proto
} // boost

#endif
