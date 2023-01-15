//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_GZIP_CODEC_HPP
#define BOOST_HTTP_PROTO_DETAIL_GZIP_CODEC_HPP

#include <boost/http_proto/detail/codec.hpp>

namespace boost {
namespace http_proto {
namespace detail {

class gzip_decoder : public codec
{
    void* zs_ = nullptr;

public:
    ~gzip_decoder();
    gzip_decoder();

    gzip_decoder(
        gzip_decoder&&) noexcept;
    gzip_decoder& operator=(
        gzip_decoder&&) = delete;

    results
    exchange(
        void* output,
        std::size_t output_size,
        void const* input,
        std::size_t input_size) override;
};

//------------------------------------------------

class gzip_encoder : public codec
{
    void* zs_ = nullptr;

public:
    ~gzip_encoder();
    gzip_encoder();

    gzip_encoder(
        gzip_encoder&&) noexcept;
    gzip_encoder& operator=(
        gzip_encoder&&) = delete;

    results
    exchange(
        void* output,
        std::size_t output_size,
        void const* input,
        std::size_t input_size) override;
};

} // detail
} // http_proto
} // boost

#endif
