//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_BROTLI_CODEC_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_BROTLI_CODEC_IPP

#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

namespace detail {

// class brotli_decoder

//------------------------------------------------

// class brotli_encoder

} // detail

//------------------------------------------------

void
serializer::
apply_param(
    brotli_decoder_t const&)
{
}

void
serializer::
apply_param(
    brotli_encoder_t const&)
{
}

//------------------------------------------------

void
parser::
apply_param(
    brotli_decoder_t const&)
{
}

} // http_proto
} // boost

#endif
