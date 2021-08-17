//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_BASIC_MESSAGE_IPP
#define BOOST_HTTP_PROTO_IMPL_BASIC_MESSAGE_IPP

#include <boost/http_proto/basic_message.hpp>

namespace boost {
namespace http_proto {

basic_message::
basic_message() = default;

//------------------------------------------------

basic_message::
basic_message(
    string_view start_line)
{
    (void)start_line;
}

} // http_proto
} // boost

#endif
