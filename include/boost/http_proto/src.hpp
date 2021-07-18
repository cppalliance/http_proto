//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_SRC_HPP
#define BOOST_HTTP_PROTO_SRC_HPP

/*

This file is meant to be included once,
in a translation unit of the program.

*/

#ifndef BOOST_HTTP_PROTO_SOURCE
#define BOOST_HTTP_PROTO_SOURCE
#endif

// We include this in case someone is
// using src.hpp as their main header file
#include <boost/http_proto.hpp>

#include <boost/http_proto/detail/rfc7230.ipp>
#include <boost/http_proto/impl/basic_parser.ipp>
#include <boost/http_proto/impl/error.ipp>
#include <boost/http_proto/impl/request_parser.ipp>
#include <boost/http_proto/impl/verb.ipp>

#endif
