//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BROTLI_HPP
#define BOOST_HTTP_PROTO_BROTLI_HPP

#include <boost/http_proto/detail/config.hpp>

namespace boost {
namespace http_proto {

#ifdef BOOST_HTTP_PROTO_DOCS

/** Constant for the brotli decoder

    This value may be passed upon construction
    of a @ref serializer or @ref parser.
*/
constexpr __implementation_defined__ brotli_decoder;

/** Constant for the gzip encoder

    This value may be passed upon construction
    of a @ref serializer or @ref parser.
*/
constexpr __implementation_defined__ brotli_encoder;

#else

struct brotli_decoder_t {};
struct brotli_encoder_t {};
constexpr brotli_decoder_t brotli_decoder{};
constexpr brotli_encoder_t brotli_encoder{};

#endif

} // http_proto
} // boost

#endif
