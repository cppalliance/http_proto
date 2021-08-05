//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_TRANSFER_PARAMETER_LIST_IPP
#define BOOST_HTTP_PROTO_IMPL_TRANSFER_PARAMETER_LIST_IPP

#include <boost/http_proto/transfer_parameter_list.hpp>
#include <boost/http_proto/ctype.hpp>

namespace boost {
namespace http_proto {

/*
    legacy list rules:
    1#element => *( "," OWS ) element *( OWS "," [ OWS element ] )
*/

char const*
transfer_parameter_list_bnf::
begin(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    return nullptr;
}

char const*
transfer_parameter_list_bnf::
increment(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    return nullptr;
}

} // http_proto
} // boost

#endif
