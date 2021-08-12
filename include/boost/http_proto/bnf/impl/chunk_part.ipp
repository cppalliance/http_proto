//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_PART_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_CHUNK_PART_IPP

#include <boost/http_proto/bnf/chunk_part.hpp>
#include <boost/http_proto/error.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
chunk_part::
parse(
    char const* start,
    char const* end,
    error_code& ec)
{
    return start;
}

} // bnf
} // http_proto
} // boost

#endif
