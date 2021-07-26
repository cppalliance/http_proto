//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DEFLATE_SERVICE_HPP
#define BOOST_HTTP_PROTO_DEFLATE_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
#endif

/** Install the deflate decoder into the specified context.
*/
BOOST_HTTP_PROTO_DECL
void
install_deflate_decoder(
    context& ctx);

/** Install the deflate encoder into the specified context.
*/
BOOST_HTTP_PROTO_DECL
void
install_deflate_encoder(
    context& ctx);

} // http_proto
} // boost

#endif
