//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_EXT_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_EXT_IPP

#include <boost/http_proto/bnf/chunk_ext.hpp>
#include <boost/http_proto/error.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
chunk_ext_elem::
parse(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    (void)start;
    (void)end;
    (void)ec;
    // VFALCO TODO
    return nullptr;
}

} // bnf
} // http_proto
} // boost

#endif
