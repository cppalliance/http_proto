//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_DEFLATE_CODEC_IPP
#define BOOST_HTTP_PROTO_IMPL_DEFLATE_CODEC_IPP

#include <boost/http_proto/deflate_codec.hpp>
#include <boost/http_proto/context.hpp>

namespace boost {
namespace http_proto {

void
install_deflate_encoder(
    context& ctx)
{
    (void)ctx;
}

void
install_deflate_decoder(
    context& ctx)
{
    (void)ctx;
}

} // http_proto
} // boost

#endif
