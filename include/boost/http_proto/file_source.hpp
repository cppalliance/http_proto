//
// Copyright (c) 2022 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_SOURCE_HPP
#define BOOST_HTTP_PROTO_FILE_SOURCE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/file.hpp>
#include <boost/http_proto/source.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

class file_source
    : public source
{
    file f_;
    std::uint64_t n_;

public:
    file_source() = delete;
    file_source(file_source const&) = delete;

    BOOST_HTTP_PROTO_DECL
    file_source(file_source&&) noexcept;

    BOOST_HTTP_PROTO_DECL
    ~file_source();

    BOOST_HTTP_PROTO_DECL
    explicit
    file_source(
        file&& f,
        std::uint64_t size =
            std::uint64_t(-1)) noexcept;

private:
    BOOST_HTTP_PROTO_DECL
    results
    on_read(
        buffers::mutable_buffer b) override;
};

} // http_proto
} // boost

#endif
